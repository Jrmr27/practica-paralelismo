# paralelism-practice
in Advanced Computer Architecture class

Creation of traces & their representation in Perfetto:
## Static
<img width="2862" height="632" alt="image" src="https://github.com/user-attachments/assets/572587e1-0d51-495f-9bb6-0238df6246c5" />

## Dynamic
<img width="2850" height="626" alt="image" src="https://github.com/user-attachments/assets/71d0c3c5-2c02-4e95-8537-820250144692" />

using "ordenar.c"

1. `gcc -O2 -fopenmp ordenar.c -o file_ordenar`

2.1. `./file_ordenar 4 static  --> mv traza.json static.json`
   
2.2. `./file_ordenar 4 dynamic  --> mv traza.json dynamic.json`

4. download json files & Perfetto [https://ui.perfetto.dev/] load of them


