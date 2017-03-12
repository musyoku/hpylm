#include "model.cpp"
using namespace std;

void test_train(){
	PyHPYLM* model = new PyHPYLM(3);
	model->load_textfile("dataset/alice.txt", 1100);
	model->set_g0(1.0 / model->get_num_types_of_words());

	for(int epoch = 1;epoch < 10;epoch++){
		model->perform_gibbs_sampling();
		double ppl = model->compute_perplexity_test();
		cout << ppl << endl;
	}
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
	for(int i = 0;i < 10;i++){
		test_train();
	}
}
