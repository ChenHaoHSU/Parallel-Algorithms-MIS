make clean
make

for i in {1..8}
do
  echo "========================"
  echo "= Input $i"
  echo "========================"
  ./bin/mis input/input$i.txt Blelloch 32
done

#for i in {8..8}
#do
  #for t in 1 2 4 8 16 32 64
  #do
    #echo "========================"
    #echo "= Threads $t"
    #echo "========================"
    #./bin/mis input/input$i.txt Luby $t
  #done
#done

