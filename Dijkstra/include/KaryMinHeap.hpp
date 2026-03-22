#ifndef KARY_MIN_HEAP
#define KARY_MIN_HEAP

#include <iostream>
#include <vector>

struct Node {
    int distance;
    int vertex; 
};

class KaryMinHeap { 
    public: 
        KaryMinHeap(int treeDegree) : k(treeDegree > 1 ? treeDegree : 2){}
        bool isEmpty(){return heap.empty();}

        void push(int dist, int v){
            heap.push_back({dist, v});
            siftUp(heap.size() - 1);
        }

        Node pop(){
            //salvo o valor do menor elemento
            Node root = heap[0];

            //salvo o ultimo elemento e excluo do vetor
            Node lastNode = heap.back(); 
            heap.pop_back(); 

            
            if(!heap.empty()){
                
                //coloco o ultimo valor como a raiz que foi retirada
                //para manter o heap balanceado
                //faco sift down para manter ordem
                heap[0] = lastNode; 
                siftDown(0);
            }
            return root; 
        } 
 

    private:
        std::vector<Node> heap; 
        int k; 

        int getParent(int i){ return (i-1)/k;}
        int getFirstChild(int i){ return (k*i + 1);}
        
        //moves a "hole" up
        void siftUp(int i) {
            Node target = heap[i];
            
            while(i>0){
                int p = getParent(i);
                if(heap[p].distance <= target.distance) break; 

                heap[i] = heap[p];
                i = p; 
            }
            heap[i] = target; 
        }

        //moves a "hole" down
        void siftDown(int i){
            Node target = heap[i];
            int size = heap.size();

            while(true){
                int firstChild = getFirstChild(i);
                
                if(firstChild >= size) break; 

                int smallestChildIdx = firstChild; 

                for (int j=0; j<k; j++){
                    int currentChild = firstChild + j; 

                    if(currentChild >= size) break; 

                    if(heap[currentChild].distance < heap[smallestChildIdx].distance){
                        smallestChildIdx = currentChild; 
                    }                    
                }

                if (heap[smallestChildIdx].distance >= target.distance) {
                    break;
                }                

                heap[i] = heap[smallestChildIdx];
                i = smallestChildIdx;    
            }
            heap[i] = target; 
        }
};

#endif