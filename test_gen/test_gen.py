import sys
import random

def gen_mis(filename, num_vertices, num_edges):
    print(f'[Info] Filename: {filename}')
    print(f'[Info] #Vertices: {num_vertices}')
    print(f'[Info] #Edges: {num_edges}')

    edges = set()
    while len(edges) < num_edges:
        v1 = random.randint(0, num_vertices - 1)
        v2 = random.randint(0, num_vertices - 1)
        if v1 == v2:
            continue
        if v1 > v2:
            v1, v2 = v2, v1
        if (v1, v2) not in edges:
            edges.add((v1, v2))
        
    with open(filename, 'w') as fout:
        fout.write(f'{num_vertices}\n')
        fout.write(f'{num_edges}\n')
        for v1, v2 in edges:
            fout.write(f'{v1} {v2}\n')

    print(f'[Info] Done!')

def main():
   random.seed(0)
   filename = sys.argv[1]
   num_vertices, num_edges = int(sys.argv[2]), int(sys.argv[3])
   gen_mis(filename, num_vertices, num_edges)

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python3", sys.argv[0], "[filename]", "[#vertices]", "[#edges]")
        exit(1)
    main()
    
