# coding: utf-8
import argparse, sys, os, time, re, codecs, random
import pandas as pd
import model

def main(args):
	try:
		os.mkdir(args.model)
	except:
		pass
	assert args.filename is not None
	assert os.path.exists(args.filename)
	assert args.train_split is not None
	assert args.ngram > 0

	hpylm = model.hpylm(args.ngram)
	hpylm.load_textfile(args.filename, args.train_split)
	print "訓練データ数:	", hpylm.get_num_train_data() 
	print "テストデータ数:	", hpylm.get_num_test_data()
	print "語彙数:		", hpylm.get_num_types_of_words()
	print "総単語数:	", hpylm.get_num_words()

	# 規定分布を設定
	hpylm.set_g0(1.0 / float(hpylm.get_num_types_of_words()))

	# グラフプロット用
	csv_likelihood = []
	csv_perplexity = []

	for epoch in xrange(1, args.epoch + 1):
		start = time.time()
		hpylm.perform_gibbs_sampling()
		hpylm.sample_hyperparameters()

		elapsed_time = time.time() - start
		sys.stdout.write("\rEpoch {} / {} - {:.3f} sec - {} nodes - {} customers".format(epoch, args.epoch, elapsed_time, hpylm.get_num_nodes(), hpylm.get_num_customers()))		
		sys.stdout.flush()
		if epoch % 10 == 0:
			log_likelihood = hpylm.compute_log_Pdataset_test() 
			perplexity = hpylm.compute_perplexity_test()
			print "\nlog_likelihood:", int(log_likelihood)
			print "perplexity:", int(perplexity)
			hpylm.save(args.model);
			# CSV出力
			csv_likelihood.append([epoch, log_likelihood])
			data = pd.DataFrame(csv_likelihood)
			data.columns = ["epoch", "log_likelihood"]
			data.to_csv("{}/likelihood.csv".format(args.model))
			csv_perplexity.append([epoch, perplexity])
			data = pd.DataFrame(csv_perplexity)
			data.columns = ["epoch", "perplexity"]
			data.to_csv("{}/perplexity.csv".format(args.model))

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("-f", "--filename", type=str, default=None, help="訓練用のテキストファイルのパス.")
	parser.add_argument("-n", "--ngram", type=int, default=3)
	parser.add_argument("-e", "--epoch", type=int, default=1000, help="総epoch.")
	parser.add_argument("-m", "--model", type=str, default="out", help="保存フォルダ名.")
	parser.add_argument("-l", "--train-split", type=int, default=None, help="テキストデータの最初の何行を訓練データにするか.")
	main(parser.parse_args())