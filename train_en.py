# coding: utf-8
import argparse, sys, os, time, re, codecs, random
import pandas as pd
import model

class stdout:
	BOLD = "\033[1m"
	END = "\033[0m"
	CLEAR = "\033[2K"

def main(args):
	try:
		os.mkdir(args.model)
	except:
		pass
	assert args.filename is not None
	assert os.path.exists(args.filename)
	assert args.train_split is not None
	assert args.hpylm_depth > 0

	hpylm = model.hpylm(args.hpylm_depth)
	hpylm.load_textfile(args.filename, args.train_split)
	print stdout.BOLD + "訓練データ数:", hpylm.get_num_train_data() 
	print "テストデータ数:", hpylm.get_num_test_data() , stdout.END

	# グラフプロット用
	csv_likelihood = []
	csv_perplexity = []

	for epoch in xrange(1, args.epoch + 1):
		start = time.time()
		hpylm.perform_gibbs_sampling()

		elapsed_time = time.time() - start
		sys.stdout.write("\rEpoch {} / {} - {:.3f} sec".format(epoch, args.epoch, elapsed_time))		
		sys.stdout.flush()
		if epoch % 100 == 0:
			print "\n"
			log_likelihood = hpylm.compute_log_Pdataset_test() 
			perplexity = hpylm.compute_perplexity_test()
			print "log_likelihood:", int(log_likelihood)
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
	parser.add_argument("-d", "--hpylm-depth", type=int, default=3, help="HPYLMの深さ.")
	parser.add_argument("-e", "--epoch", type=int, default=1000000, help="総epoch.")
	parser.add_argument("-m", "--model", type=str, default="out", help="保存フォルダ名.")
	parser.add_argument("-u", "--unknown-threshold", type=int, default=0, help="出現回数がこの値以下の単語は<unk>に置き換える.")
	parser.add_argument("-l", "--train-split", type=int, default=None, help="テキストデータの最初の何行を訓練データにするか.")
	main(parser.parse_args())