
#include <string>
#include <iostream>
#include <thread>
#include <barrier>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include <ff/ff.hpp>


using namespace ff;
using namespace std;

barrier bar{2};
atomic_bool managerstop{false};

struct Source: ff_node_t<long> {

	const int ntasks;
	cpu_set_t cpuset;

    Source(const int ntasks, const cpu_set_t cpu_set):ntasks(ntasks), cpuset(cpu_set) {}

	int svc_init() {
    	// // set the affinity for the current thread
    	if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
    		cout << "Errore durante l'impostazione dell'affinità della CPU." << endl;
    	}
		bar.arrive_and_wait();
		return 0;
	}
	long* svc(long*) {
    	assert(sched_getcpu()== 0 || sched_getcpu()== 16);
        for(long i=1;i<=ntasks;++i) {

			ticks_wait(1000);
            ff_send_out((long*)i);
        }
        return EOS;
    }

};

struct Stage: ff_node_t<long> {

	long workload;
	cpu_set_t cpuset;

	Stage(long workload, const cpu_set_t cpu_set):workload(workload), cpuset(cpu_set) {}

	int svc_init() {
		// set the affinity for the current thread
		if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
			cout << "Errore durante l'impostazione dell'affinità della CPU." << endl;
		}
		return 0;
	}

    long* svc(long*in) {
		assert(sched_getcpu()== 0 || sched_getcpu()== 16);
		ticks_wait(workload);
        return in;
    }

};

struct Sink: ff_node_t<long> {

	size_t counter=0;
	cpu_set_t cpuset;

	Sink(cpu_set_t cpu_set): cpuset(cpu_set){}

	int svc_init() {
		// set the affinity for the current thread
		if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
			cout << "Errore durante l'impostazione dell'affinità della CPU." << endl;
		}
		return 0;
	}

    long* svc(long*) {
		assert(sched_getcpu()== 0 || sched_getcpu()== 16);
		ticks_wait(1000);
        ++counter;
        return GO_ON;
    }

	void svc_end() {
		std::printf("Sink finished\n");
		managerstop=true;
	}
};

class Manager {

private:
    ff_pipeline& pipe;
    chrono::milliseconds interval;
    ofstream outFile;
    condition_variable cv;
    mutex cv_m;

    /**
     *
     * @return the pipeline operators' output queue length.
     */
    vector<unsigned long>* get_operators_queues_lengths () {
        const svector<ff_node*> nodes = pipe.get_pipeline_nodes();
        vector<unsigned long>* queue_lenghts = new vector<unsigned long> (nodes.size()-1);

        std::unique_lock<std::mutex> lk(cv_m);
        cv.wait_for(lk, interval, [this] { return managerstop.load(); });

        if (managerstop.load()) {
            delete queue_lenghts;
            return nullptr;
        }

        for(size_t i=0;i<(nodes.size()-1); ++i) {
            svector<ff_node*> in;
            nodes[i]->get_out_nodes(in);  // retrieve of the output nodes of the current node
            (*queue_lenghts)[i] = in[0]->get_out_buffer()->length();
            outFile << "node" << (i + 1) << " qlen=" << in[0]->get_out_buffer()->length() << std::endl;
        }
        outFile << "-------" << std::endl;

        return queue_lenghts;
    }

    /**
     *
     * @param queue_lengths pipeline operators' output queue length.
     * @return the logical priority for each operator.
     */
    vector<double>* get_logical_priority(vector<unsigned long>* queue_lengths) {
        vector<double>* logical_priorities = new vector<double>(queue_lengths->size());

        /*
         * Stuff
         *
         */

        return logical_priorities;
    }

	vector<int>* translation (vector<double>* logical_priority) {
    	vector<int>* concrete_priorities = new vector<int>(logical_priority->size());

    	/*
		 * Stuff
		 *
		 */

    	return concrete_priorities;
    }

	void apply_priorities(vector<int>* concrete_priorities) {

		svector<ff_node*> pipe_nodes = pipe.get_pipeline_nodes();

    	assert (pipe_nodes.size() == concrete_priorities->size());

	    for(size_t i = 0; i < concrete_priorities->size()-1; i++) {

	    }
    }

public:
    Manager(ff_pipeline& p, std::chrono::milliseconds i)
        : pipe(p), interval(i), outFile("logs/out.log") {
        if (!outFile.is_open()) {
            cerr << "Error opening output file" << endl;
            exit(EXIT_FAILURE);
        }
    }

    void run() {
        bar.arrive_and_wait();
        std::printf("manager started\n");

        while(!managerstop.load()) {
            // Retrieves of the operators' queues lengths.
            vector<unsigned long>* queue_lenghts = this->get_operators_queues_lengths();

            if(managerstop) {
                std::printf("manager completed\n");
                delete queue_lenghts;
                return;
            }


        }



        std::printf("manager completed\n");
    }



};

int main(int argc, char* argv[]) {

    // default arguments
    size_t ntasks = 10000;
    size_t nnodes = 2;
	int interval_ms = 100; // Default interval (100 ms)

	if (argc > 1) {
		if (argc != 4) {
			error("use: %s ntasks nnodes interval_ms\n", argv[0]);
			return -1;
		}
		ntasks = std::stol(argv[1]);
		nnodes = std::stol(argv[2]);
		interval_ms = std::stoi(argv[3]);
	}

	/* Definition of cpuset. Used for restrict the cpus on which
	 * the threads of the pipeline can be scheduled. */
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(0, &cpuset);
	CPU_SET(16, &cpuset);
	//CPU_SET(32, &cpuset);
	//CPU_SET(48, &cpuset);

    Source first(ntasks, cpuset);
    Sink   last(cpuset);

	ff_pipeline pipe;
	pipe.add_stage(&first);
	for(size_t i=1;i<=nnodes;++i)
		pipe.add_stage(new Stage(2000*i, cpuset), true);
	pipe.add_stage(&last);

	// setta tutte le code a bounded di capacità 10
	// pipe.setXNodeInputQueueLength(10, true);

	// lancio il thread manager
	Manager manager(pipe, std::chrono::milliseconds(interval_ms));
	std::thread managerThread([&manager]() { manager.run(); });


	//std::thread th(manager, std::ref(pipe));

	// executing the pipe
    if (pipe.run_and_wait_end()<0) {
        error("running pipeline\n");
        return -1;
    }
	std::printf("pipe done\n");
	managerThread.join();
	std::printf("manager done\n");
	cout<< "Pipeline execution time: " << pipe.ffTime() << " ms"<< endl;

	return 0;
}