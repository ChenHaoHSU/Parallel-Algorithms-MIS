make clean
make
for i in {1..10}
do
  echo "========================"
  echo "= Input $i"
  echo "========================"
  ./bin/mis input/input$i.txt 
done