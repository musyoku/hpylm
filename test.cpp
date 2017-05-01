#include "model.cpp"
using namespace std;

void test_train(){
    string dirname = "out";
	PyHPYLM* model = new PyHPYLM(3);
	model->load_textfile("dataset/wiki.txt", 0.95);
	model->set_g0(1.0 / model->get_num_types_of_words());

	for(int epoch = 1;epoch < 1000;epoch++){
		model->perform_gibbs_sampling();
		// model->sample_hyperparameters();
		double ppl = model->compute_perplexity_test();
		double log_likelihood = model->compute_log_Pdataset_test();
		cout << ppl << ", " << log_likelihood << endl;
	}
	printf("# of nodes: %d\n", model->_hpylm->get_num_nodes());
	printf("# of customers: %d\n", model->_hpylm->get_num_customers());
	printf("# of tables: %d\n", model->_hpylm->get_num_tables());
	printf("stop count: %d\n", model->_hpylm->get_sum_stop_counts());
	printf("pass count: %d\n", model->_hpylm->get_sum_pass_counts());
	model->save(dirname);
	model->load(dirname);
	printf("# of nodes: %d\n", model->_hpylm->get_num_nodes());
	printf("# of customers: %d\n", model->_hpylm->get_num_customers());
	printf("# of tables: %d\n", model->_hpylm->get_num_tables());
	printf("stop count: %d\n", model->_hpylm->get_sum_stop_counts());
	printf("pass count: %d\n", model->_hpylm->get_sum_pass_counts());

	model->remove_all_data();
	printf("# of nodes: %d\n", model->_hpylm->get_num_nodes());
	printf("# of customers: %d\n", model->_hpylm->get_num_customers());
	printf("# of tables: %d\n", model->_hpylm->get_num_tables());
	printf("stop count: %d\n", model->_hpylm->get_sum_stop_counts());
	printf("pass count: %d\n", model->_hpylm->get_sum_pass_counts());
	delete model;
}

int main(int argc, char *argv[]){
	for(int i = 0;i < 1;i++){
		test_train();
	}
}
