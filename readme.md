## A Hierarchical Bayesian Language Model based on Pitman-Yor Processes

- [A Hierarchical Bayesian Language Model based on Pitman-Yor Processes](http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/acl2006.pdf)
- [A Bayesian Interpretation of Interpolated Kneser-Ney](http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/hpylm.pdf) 
- [実装について](http://musyoku.github.io/2016/07/26/A_Hierarchical_Bayesian_Language_Model_based_on_Pitman-Yor_Processes/)

## 動作環境

- C++11
- Boost 1.6

## ビルド

```
make install
```

## 学習

`-f`でファイルを指定、`-l`で何行を訓練データにするかを指定します。

```
python train_en.py -f dataset/alice.txt -l 1100
python train_en.py -f dataset/wiki.txt -l 45000
```

## 生成

```
python generate.py
```

生成結果

```
alice , who seemed to be a little , the march hare . 
didn't know it to be a person of authority over alice . 
not open any of them were animals , and drew the back . 
mock turtle , but then , and was just going to remark . 
her something about the same year for such dainties would not join the dance . 
's the first figure , said the last few minutes , and held it out to be trampled under its feet , they gave him two , they were all shaped like ears and the baby the fire , licking her paws and washing her face and she did not seem to dry me at all , he said , turning to alice , as he spoke . 
might just as well say that 's a mineral , i should think you could see her . 
she had been looking over his shoulder as he said to herself , not much surprised at this . 
were a duck with its arms folded , quietly smoking a long breath , and though this was not a mile high , said the gryphon , the queen . 
cat . 
are you content now ? said alice i can't help it , you know . 
think i must be off , and yet it 's pleased so far , thought alice i can't remember half of fright and half of anger , and went on , and that 's the first figure , said the footman . 
she was a little bit , and went on at last the caterpillar decidedly , and that 's quite enough i hope i shan't go , and the executioner myself , said alice . 
bright brass plate with the other . 
's a large dish of tarts upon it . 
whatever ? persisted the king . 
quite plainly through the little door , she ran off , and four times seven is oh , you are old , father william replied to his ear . 
whatever ? persisted the king . 
might like to be a caucus race . 
believe i can listen all day about it . 
the tide rises and sharks are around , his voice and alice looked all round the court . 
queen was in march . 
it won't be raving mad at least not so mad as it happens and if it makes rather a handsome pig , i think you'd better not talk ! said alice . 
with her head impatiently , and the baby the cook and the gryphon . 
all can , said the hatter . 
your verdict , the king and queen of hearts , he said to herself , and dishes . 
, alas ! it was indeed . 
! that will be a footman because he was speaking , and alice 's shoulder , and the jury , and the words a little ledge of rock , and alice was a general clapping of hands at this , she thought of herself , as he shook his grey locks , i must be ! thought alice . 
reason is , said the mock turtle in the distance . 
she spoke . 
's very like a tunnel for some minutes the whole party swam to the shore . 
ootiful soo oop ! 
, my dear , and i've tried the little golden key and hurried off , said the march hare went on . 
a few minutes , and she was , and thought it would be quite as safe to stay in here ? 
something important to say . 
not a moment that it might not escape again , said alice . 
, that she tipped over the jury box , and both the hedgehogs and in another moment , and thought it would be a book of rules for shutting people up like a serpent and there 's a very humble tone , and then unrolled the parchment scroll , and i could tell you my adventures beginning from this side of what ? 
better not do that again ! cried the mouse was speaking , and said , and then she found herself safe in a very fine day ! why , i can find out the proper way of expressing yourself . 
gryphon said , for she had to do . 
```