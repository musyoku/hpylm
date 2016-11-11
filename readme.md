## HPYLM

C++によるHierarchical Pitman-Yor Language Modelの実装と文字n-gramの学習例です。

以下の論文をもとに実装を行っています。

- [A Hierarchical Bayesian Language Model based on Pitman-Yor Processes](http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/acl2006.pdf)
- [A Bayesian Interpretation of Interpolated Kneser-Ney](http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/hpylm.pdf) 

素組みしただけの愚直な実装ですので高速化などは考えていません。

[この記事](http://musyoku.github.io/2016/07/26/A_Hierarchical_Bayesian_Language_Model_based_on_Pitman-Yor_Processes/)で実装したプログラムになります。

Pythonラッパーもあります。

- [python-vpylm](https://github.com/musyoku/vpylm-python)

## 動作環境

- C++11
- Boost 1.6

この実装は以下のコードを含んでいます。

- [c_printf](https://github.com/Gioyik/c_printf)

## 実行

### コンパイル

```
make hpylm
```

### 学習

実行前に学習用テキストデータを`train.txt`に保存し、適当なディレクトリに入れておきます。

またモデル保存用のディレクトリ`model`もあらかじめ作成しておいてください。

実行は

```
./hpylm -t alice/train.txt -n 3
```

のようにn-gramを指定して行います。

### 結果

```
alice/train.txtを読み込みました（899行）
VPYLMを初期化しています ...
G0 <- 0.000343
Epoch 1 / 100 - 6915.4 lps - 16.060 ppl - 18072 nodes - 83251 customers
Epoch 2 / 100 - 6862.6 lps - 16.150 ppl - 18072 nodes - 83259 customers
Epoch 3 / 100 - 6709.0 lps - 16.380 ppl - 18072 nodes - 83194 customers
Epoch 4 / 100 - 6610.3 lps - 16.126 ppl - 18072 nodes - 83238 customers
Epoch 5 / 100 - 6562.0 lps - 16.443 ppl - 18072 nodes - 83188 customers
Epoch 6 / 100 - 6709.0 lps - 16.233 ppl - 18072 nodes - 83191 customers
Epoch 7 / 100 - 6659.3 lps - 16.148 ppl - 18072 nodes - 83176 customers
Epoch 8 / 100 - 6862.6 lps - 15.987 ppl - 18072 nodes - 83149 customers
Epoch 9 / 100 - 6659.3 lps - 15.884 ppl - 18072 nodes - 83119 customers
Epoch 10 / 100 - 6562.0 lps - 16.022 ppl - 18072 nodes - 83091 customers
Epoch 11 / 100 - 6562.0 lps - 15.629 ppl - 18072 nodes - 83117 customers
Epoch 12 / 100 - 6709.0 lps - 15.953 ppl - 18072 nodes - 83083 customers
Epoch 13 / 100 - 6610.3 lps - 16.546 ppl - 18072 nodes - 83096 customers
Epoch 14 / 100 - 6610.3 lps - 16.579 ppl - 18072 nodes - 83144 customers
Epoch 15 / 100 - 6610.3 lps - 15.918 ppl - 18072 nodes - 83197 customers
Epoch 16 / 100 - 6610.3 lps - 16.209 ppl - 18072 nodes - 83274 customers
Epoch 17 / 100 - 5800.0 lps - 16.315 ppl - 18072 nodes - 83225 customers
Epoch 18 / 100 - 5654.1 lps - 16.285 ppl - 18072 nodes - 83241 customers
Epoch 19 / 100 - 6157.5 lps - 16.514 ppl - 18072 nodes - 83276 customers
Epoch 20 / 100 - 5800.0 lps - 16.045 ppl - 18072 nodes - 83271 customers
Epoch 21 / 100 - 6243.1 lps - 16.270 ppl - 18072 nodes - 83208 customers
Epoch 22 / 100 - 6562.0 lps - 16.187 ppl - 18072 nodes - 83246 customers
Epoch 23 / 100 - 6467.6 lps - 16.172 ppl - 18072 nodes - 83302 customers
Epoch 24 / 100 - 6514.5 lps - 15.988 ppl - 18072 nodes - 83328 customers
Epoch 25 / 100 - 6467.6 lps - 16.377 ppl - 18072 nodes - 83198 customers
Epoch 26 / 100 - 6467.6 lps - 16.036 ppl - 18072 nodes - 83255 customers
Epoch 27 / 100 - 6467.6 lps - 16.223 ppl - 18072 nodes - 83191 customers
Epoch 28 / 100 - 6467.6 lps - 16.090 ppl - 18072 nodes - 83256 customers
Epoch 29 / 100 - 5875.8 lps - 16.082 ppl - 18072 nodes - 83143 customers
Epoch 30 / 100 - 5079.1 lps - 16.353 ppl - 18072 nodes - 83248 customers
Epoch 31 / 100 - 5953.6 lps - 16.359 ppl - 18072 nodes - 83329 customers
Epoch 32 / 100 - 6467.6 lps - 16.275 ppl - 18072 nodes - 83184 customers
Epoch 33 / 100 - 6514.5 lps - 16.554 ppl - 18072 nodes - 83249 customers
Epoch 34 / 100 - 6331.0 lps - 16.298 ppl - 18072 nodes - 83259 customers
Epoch 35 / 100 - 6562.0 lps - 16.316 ppl - 18072 nodes - 83389 customers
Epoch 36 / 100 - 6562.0 lps - 16.254 ppl - 18072 nodes - 83266 customers
Epoch 37 / 100 - 6759.4 lps - 16.218 ppl - 18072 nodes - 83298 customers
Epoch 38 / 100 - 6610.3 lps - 15.866 ppl - 18072 nodes - 83236 customers
Epoch 39 / 100 - 6610.3 lps - 15.995 ppl - 18072 nodes - 83277 customers
Epoch 40 / 100 - 6610.3 lps - 16.298 ppl - 18072 nodes - 83282 customers
Epoch 41 / 100 - 6610.3 lps - 16.147 ppl - 18072 nodes - 83167 customers
Epoch 42 / 100 - 5654.1 lps - 16.070 ppl - 18072 nodes - 83194 customers
Epoch 43 / 100 - 5953.6 lps - 16.397 ppl - 18072 nodes - 83192 customers
Epoch 44 / 100 - 5875.8 lps - 15.904 ppl - 18072 nodes - 83165 customers
Epoch 45 / 100 - 5689.9 lps - 15.989 ppl - 18072 nodes - 83190 customers
Epoch 46 / 100 - 5993.3 lps - 16.181 ppl - 18072 nodes - 83260 customers
Epoch 47 / 100 - 5914.5 lps - 16.051 ppl - 18072 nodes - 83291 customers
Epoch 48 / 100 - 5875.8 lps - 16.128 ppl - 18072 nodes - 83246 customers
Epoch 49 / 100 - 5800.0 lps - 16.182 ppl - 18072 nodes - 83109 customers
Epoch 50 / 100 - 5914.5 lps - 15.979 ppl - 18072 nodes - 83111 customers
Epoch 51 / 100 - 5914.5 lps - 15.789 ppl - 18072 nodes - 83064 customers
Epoch 52 / 100 - 5837.7 lps - 15.955 ppl - 18072 nodes - 83147 customers
Epoch 53 / 100 - 5993.3 lps - 16.082 ppl - 18072 nodes - 83197 customers
Epoch 54 / 100 - 5914.5 lps - 15.616 ppl - 18072 nodes - 83143 customers
Epoch 55 / 100 - 5837.7 lps - 16.341 ppl - 18072 nodes - 83159 customers
Epoch 56 / 100 - 5800.0 lps - 16.281 ppl - 18072 nodes - 83220 customers
Epoch 57 / 100 - 5875.8 lps - 16.289 ppl - 18072 nodes - 83237 customers
Epoch 58 / 100 - 5837.7 lps - 16.542 ppl - 18072 nodes - 83278 customers
Epoch 59 / 100 - 5762.8 lps - 16.274 ppl - 18072 nodes - 83327 customers
Epoch 60 / 100 - 6033.6 lps - 16.254 ppl - 18072 nodes - 83299 customers
Epoch 61 / 100 - 5953.6 lps - 15.884 ppl - 18072 nodes - 83201 customers
Epoch 62 / 100 - 6033.6 lps - 15.885 ppl - 18072 nodes - 83143 customers
Epoch 63 / 100 - 5993.3 lps - 16.450 ppl - 18072 nodes - 83245 customers
Epoch 64 / 100 - 5993.3 lps - 16.475 ppl - 18072 nodes - 83248 customers
Epoch 65 / 100 - 5618.8 lps - 16.344 ppl - 18072 nodes - 83193 customers
Epoch 66 / 100 - 5875.8 lps - 16.187 ppl - 18072 nodes - 83247 customers
Epoch 67 / 100 - 5914.5 lps - 15.874 ppl - 18072 nodes - 83225 customers
Epoch 68 / 100 - 5914.5 lps - 16.231 ppl - 18072 nodes - 83245 customers
Epoch 69 / 100 - 5914.5 lps - 15.995 ppl - 18072 nodes - 83211 customers
Epoch 70 / 100 - 5914.5 lps - 16.067 ppl - 18072 nodes - 83269 customers
Epoch 71 / 100 - 5953.6 lps - 16.205 ppl - 18072 nodes - 83288 customers
Epoch 72 / 100 - 5953.6 lps - 16.088 ppl - 18072 nodes - 83349 customers
Epoch 73 / 100 - 5875.8 lps - 16.106 ppl - 18072 nodes - 83313 customers
Epoch 74 / 100 - 5953.6 lps - 16.189 ppl - 18072 nodes - 83309 customers
Epoch 75 / 100 - 3511.7 lps - 16.233 ppl - 18072 nodes - 83300 customers
Epoch 76 / 100 - 6467.6 lps - 16.192 ppl - 18072 nodes - 83246 customers
Epoch 77 / 100 - 6610.3 lps - 16.064 ppl - 18072 nodes - 83298 customers
Epoch 78 / 100 - 6610.3 lps - 15.942 ppl - 18072 nodes - 83141 customers
Epoch 79 / 100 - 6862.6 lps - 15.912 ppl - 18072 nodes - 83125 customers
Epoch 80 / 100 - 6759.4 lps - 16.085 ppl - 18072 nodes - 83172 customers
Epoch 81 / 100 - 6562.0 lps - 16.288 ppl - 18072 nodes - 83317 customers
Epoch 82 / 100 - 6709.0 lps - 16.379 ppl - 18072 nodes - 83253 customers
Epoch 83 / 100 - 6810.6 lps - 16.118 ppl - 18072 nodes - 83333 customers
Epoch 84 / 100 - 6759.4 lps - 16.113 ppl - 18072 nodes - 83241 customers
Epoch 85 / 100 - 6610.3 lps - 16.103 ppl - 18072 nodes - 83159 customers
Epoch 86 / 100 - 6421.4 lps - 15.822 ppl - 18072 nodes - 83316 customers
Epoch 87 / 100 - 6759.4 lps - 15.725 ppl - 18072 nodes - 83274 customers
Epoch 88 / 100 - 5022.3 lps - 16.178 ppl - 18072 nodes - 83254 customers
Epoch 89 / 100 - 5288.2 lps - 15.995 ppl - 18072 nodes - 83313 customers
Epoch 90 / 100 - 5726.1 lps - 15.864 ppl - 18072 nodes - 83190 customers
Epoch 91 / 100 - 5549.4 lps - 16.043 ppl - 18072 nodes - 83222 customers
Epoch 92 / 100 - 5257.3 lps - 15.824 ppl - 18072 nodes - 83213 customers
Epoch 93 / 100 - 5875.8 lps - 15.767 ppl - 18072 nodes - 83166 customers
Epoch 94 / 100 - 5800.0 lps - 16.171 ppl - 18072 nodes - 83174 customers
Epoch 95 / 100 - 5837.7 lps - 16.018 ppl - 18072 nodes - 83248 customers
Epoch 96 / 100 - 5875.8 lps - 16.128 ppl - 18072 nodes - 83313 customers
Epoch 97 / 100 - 5481.7 lps - 16.145 ppl - 18072 nodes - 83446 customers
Epoch 98 / 100 - 5953.6 lps - 16.062 ppl - 18072 nodes - 83314 customers
Epoch 99 / 100 - 5914.5 lps - 16.081 ppl - 18072 nodes - 83354 customers
Epoch 100 / 100 - 6115.6 lps - 15.919 ppl - 18072 nodes - 83219 customers
2
18072
83219
35707
71414
文章を生成しています ...
" I never heard it say to itself , " But she went on growing , and in that ridiculous fashion . " 
" You are old , " to go , and there . " I only wish people knew that it would be quite as much as she spoke , but looked at Alice , " Ive heard something splashing about in the window , she ran ; but I know something interesting is sure to make out which were the two creatures got so close to her that she had grown so large a house , and very nearly at the bottom of the players , and noticed that they would not join the dance . So she began , in a wondering tone . And yet I wish I hadn't cried so much ! " said the Cat . " 
" I'm a  " 
While the Owl had the dish as its share of the thing Mock Turtle . " Fetch me my gloves this moment , and the game was going to be almost out of the tale was something like it put more simply   I gave her one , time for dinner ! " Alice replied very gravely . " 
" Tut , tut , child ! " she said to herself , " but it had no idea what to say  How doth the little magic bottle had now had its full effect , and though this was not easy to take more than that . Then it wasn't very civil of you to leave off , thinking while she ran off , and she , " said Alice . " Ive heard something splashing about in a long way . 
So she set off at once . " 
" What for ? " 
The baby grunted again , the Duchess to play croquet ? " 
" And in she went down to her that she began thinking over other children she knew she had to ask . " 
" Oh , do let me hear the words all coming different , and hurried off . 
" What was that it was too late to wish that ! " said the Mock Turtle , " What was that ? " said the Footman went on , without even looking round the court ! Suppress him ! Off with her head impatiently , and said to the door of the trial done , " you shouldn't talk , " said the Queen , who seemed to be no chance of her head . " I wish I hadn't cried so much contradicted in her pocket , and she had caught the baby was sneezing on the bank , and she is only a pack of cards : the roses . " Let me see : I'll give them a new pair of white kid gloves , and some of the window , and then nodded . " 
" It's the stupidest tea - tray in the house of the month is it I can't remember things as I used to say it over ) "  change lobsters , and broke to pieces . 
" I won't , then , " if you like the look of it : there were no arches left , and dishes . The Hatter was the first to speak again . 
So they went on , " said the Cat . 
" What is the capital of Paris , and then ; such as , that it was , and began to repeat it , " said the Hatter , and the great concert given by the English coast you find a number of changes she had not ! " and read as follows 
" Never mind ! " and their slates ; " very ill . " 
" Well , I don't see , as it was growing , and must know better'; and this Alice thought to herself , " said Alice to herself , and making quite a chorus of " There isn't any , " she thought , " and then all the time she found herself in the other , and said , " It's the thing yourself , " and that's all the jurymen on to her that she began :  
" What was that she had found the fan and the jury , in a shrill , passionate voice . 
" Not yet , " said Alice in a great many more than that . " 
" I can't remember , " Alice replied thoughtfully . " And then , when the Queen , who was reading , but she did not get hold of anything , but her voice . 
" Well , Ive made up my mind about it ! " said Alice . 
" What I was when I was thinking I should understand that better , " said the Caterpillar . 
Then they both bowed low . 
Who would not join the dance . 
" No , no ! The Queen turned crimson with fury , and make out that she had put the hookah out of the crowd below , and the Panther received knife and fork with a little pattering of feet in the court , she noticed that they were filled with cupboards and book - shelves ; here and there was no more to come out among the trees as well as the things between whiles . " 
Alice looked all round her head ! Off with her head . " 
For anything tougher than suet ; 
" I shall be a book of rules for shutting people up like a telescope . " And she thought it over a little bit of mushroom , and the Queen's shrill cries to the door ; so , " Father William , ' thought Alice . 
" What a curious dream , dear ! I suppose ? " Alice replied very solemnly . 
First it marked out a race - course , " said the King said , and , by the way down here , " said Alice in a fight with another dig of her own courage . " 
" Serpent ! " on and off , and washing  extra .  " 
This was not a moment that it was too much of a large ring , and made a memorandum of the court ! " 
" I don't like the three gardeners at it : " it's very interesting dance to watch them , and the roof bear ?  It was the best of educations  in fact , a song ? " he said to herself " It's a friend of mine , " in fact , I suppose it doesn't matter which way you have to beat time when she thought , and had just begun to repeat it , " said Alice loudly . " 
" There's no sort of lullaby to it , " said the March Hare said in a moment like a Jack - in - the - way things had happened . 
Just then she walked off , thinking while she remembered having seen in her head ! " cried Alice hastily ; " and I'll tell him  it was the matter worse . You grant that ? " 
" What is the same thing as  I eat  is the same thing as  I eat  is the capital of Paris , and making quite a crowd of little cartwheels , and began picking them up , and fighting for the accident of the house , and had just begun " Well , I'd hardly finished the guinea - pigs , who had not attended to this last remark , " said the Queen said severely " Who is this ? " said the King , the Queen , " and she was out of this elegant thimble'; and , as it could go , for she was rather doubtful whether she ought to tell them something more . " I don't believe you do . 
" A nice muddle their slates'll be in Bill's place for a long way . 
" Ive seen hatters before , " till its ears have come here . " 
" I don't think they play at all a pity !  
It was the first verse , ' the Hatter were having tea at it again ! " she thought , " and in his throat , " said the Cat . " That's the first position in which the words have got into the darkness as hard as it went , " Come on ! " and the baby , and no more to do so . " Alice replied thoughtfully . " 
" They lived on treacle , " What a pity . I wonder if I only wish it was over , and a fall as this before , and I do , " Alice whispered to the jury , and began whistling . 
" Well , if he had to kneel down on their hands and feet , to sing you a present of everything Ive said as yet had any sense , and she tried the roots of trees , and sighing . 
" Boots and shoes ! " 
" Oh ! the little golden key was lying under the sea , though she very seldom followed it ) , and all of them . " 
Here was another puzzling question ; and when she looked down at her , " you first form into a pig , I suppose ? " 
" Why , what a wonderful dream it had struck her foot slipped , and to hear the very middle of the sea . " 
" If I eat or drink anything ; then , ' the Hatter was the White Rabbit as he spoke , and all dripping wet , cross , and noticed that one of the other , and very neatly and simply arranged ; the great wonder is , look at them , and seemed to her ear . 
" It's all his fancy , that you couldn't cut off , " so Alice went on just as she could not , could not , " and I shall be late ! " and then they wouldn't be so easily offended , you see , so that altogether , like a telescope . " The Queen smiled and passed on . 
Alice waited a little nervous about it ! Oh , my dear , how is it directed to ? " 
" I never understood what it was only sobbing , " Alice hastily replied ; " I wish you would have this cat removed ! " And what an ignorant little girl or a watch to take more than nothing . 
```

