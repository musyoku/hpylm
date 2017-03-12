# coding: utf-8
import argparse, sys, os, time, re, codecs, random
import model

def main(args):
	assert os.path.exists(args.model)
	hpylm = model.hpylm()
	hpylm.load(args.model)
	for n in xrange(args.num_generate):
		print hpylm.generate_sentence()

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("-n", "--num-generate", type=int, default=100, help="生成する文章の数.")
	parser.add_argument("-m", "--model", type=str, default="out", help="モデルが保存されているフォルダ名.")
	main(parser.parse_args())