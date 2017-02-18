# test for loop

. cdtango

raspistill -p '1200,500,640,480' -t 0 -w 640 -h 480 -vf  

for ((i=1;i<=20;i=i+1))
do
	raspistill -p '1200,500,640,480' -t 500 -w 640 -h 480 -vf -hf -o $i.jpg 
done 