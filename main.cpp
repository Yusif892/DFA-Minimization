#include <iostream>
using namespace std;
#define endl '\n'
#define ALPHA 26

template <class T>
class list 
{
private:
    T* items;
    int count;
    int capacity;

    void grow()
    {
        int new_capacity = capacity == 0 ? 1 : capacity * 2;
        T* new_items = new T[new_capacity];
        for (int i = 0; i < count; ++i)
            new_items[i] = items[i];
        delete[] items;
        items = new_items;
        capacity = new_capacity;
    }

public:
    list() : items(nullptr), count(0), capacity(0) {}


    ~list()
    {
        delete[] items;
    }

    T& operator[](int index)
    {
        return items[index];
    }

    void push(T item)
    {
        if (count == capacity)
            grow();
        items[count] = item;
        ++count;
    }

    int size() const
    {
        return count;
    }

    bool empty() const
    {
        return count == 0;
    }

};

struct Pair
{
    int p, q;

    Pair() : p(0), q(0) {}

    Pair(int a, int b)
    {
        if (a < b) {p = a; q = b;}
        else {p = b; q = a;}
    }
};

struct DFA
{
    int num_states;
    list<int> **adj;
    list<int> **reversed_graph;
    bool **equivalent;
    bool *is_accepting;
    list<Pair> non_equivalent_pairs;

    DFA(int n) : num_states(n)
    {
        adj = new list<int>* [n];
        reversed_graph = new list<int>* [n];
        equivalent = new bool*[n];
        is_accepting = new bool[n];
        for (int i = 0; i < n; i++)
        {
            adj[i] = new list<int> [ALPHA];
            reversed_graph[i] = new list<int> [ALPHA];
            equivalent[i] = new bool[n];
            is_accepting[i] = false;
        }
    }

    ~DFA()
    {
        for (int i = 0; i < num_states; i++)
        {
            delete[] adj[i];
            delete[] reversed_graph[i];
            delete[] equivalent[i];
        }
        delete[] adj;
        delete[] reversed_graph;
        delete[] equivalent;
        delete[] is_accepting;
    }

    DFA(const DFA &) = delete;

    DFA& operator=(const DFA &) = delete;

    
    void add_transition(int from, int to, char sym)
    {
        adj[from][sym - 'a'].push(to);
        reversed_graph[to][sym - 'a'].push(from);
    }

    
    void set_accepting(int state)
    {
        is_accepting[state] = true;
    }
    
    
    void build_nonequivalent_list() 
    { 
        /*
            fills the @non_eq list with the non equivalent pairs
            also builds the @equivalent matrix the determines if two states are equivalents
        */
    
        equivalent = new bool* [num_states];
        for (int i = 0; i < num_states; i++)
            equivalent[i] = new bool [num_states];
        
        
        // marking each pair of accepting and non-accepting state as not equivalent
        for (int i = 0; i < num_states; i++) 
            // stop at j <= i so we visit each pair only once as the matrix is symmetric
            for (int j = 0; j <= i; j++) {
                equivalent[i][j] = (is_accepting[i] == is_accepting[j]);
                equivalent[j][i] = equivalent[i][j];
                if (!equivalent[i][j])
                    non_equivalent_pairs.push(Pair(i,j));
            }
    }
    
    
    void mark_dependencies() 
    {
        /*
        iterates over the @non_equivalent_pairs, constructs dependencies of each pair from the reversed graph
        and marks their states as non equivalents. 
        when two states p,q are found inequivalent, the @equivalent[p][q] is set false

        for example if p,q are not equivalents and r --a-> p, s --a-> q, then r,s are marked as non-equivalents
        then r,s is pushed to the list as a pair to check their dependencies 
        
        */
    
        for (int idx = 0; idx < non_equivalent_pairs.size(); ++idx)
        {
            Pair cur = non_equivalent_pairs[idx];
            int p = cur.p, q = cur.q;
    
            for (int a = 0; a < ALPHA; ++a)
            {
                list<int> &pred_p = reversed_graph[p][a]; // go to p by a
                list<int> &pred_q = reversed_graph[q][a]; // go to q b a
    
                for (int i = 0; i < pred_p.size(); ++i)
                {
                    int r = pred_p[i];
                    for (int j = 0; j < pred_q.size(); ++j)
                    {
                        int s = pred_q[j];
                        if (r == s) continue;
    
                        if (equivalent[r][s])
                        {
                            equivalent[r][s] = false;
                            equivalent[s][r] = false;
                            non_equivalent_pairs.push(Pair(r, s));
                        }
                    }
                }
            }
        }
    }
    
    
    int* build_representatives()
    {
        int *rep = new int[num_states];
        for (int i = 0; i < num_states; i++)
        {
            rep[i] = i;
            for (int j = 0; j < i; j++)
                if (equivalent[i][j]) { rep[i] = j; break; }
        }
        return rep;
    }

    
    void print_class(int r, const int *rep)
    {
        cout << "{";
        bool first = true;
        for (int i = 0; i < num_states; i++)
            if (rep[i] == r)
            {
                if (!first) cout << ",";
                cout << i;
                first = false;
            }
        cout << "}";
    }
    
    
    void print_minimized(const int *rep)
    {
        int m = 0;
        for (int i = 0; i < num_states; i++)
            if (rep[i] == i) m++;
    
        cout << "Number of states: " << m << endl;
    
        cout << "States: ";
        for (int i = 0; i < num_states; i++)
            if (rep[i] == i) { print_class(i, rep); cout << " "; }
        cout << endl;
    
        cout << "Accepting states: ";
        for (int i = 0; i < num_states; i++)
            if (rep[i] == i && is_accepting[i]) { print_class(i, rep); cout << " "; }
        cout << endl;
    
        cout << "Transitions (from -- to -- edge):" << endl;
        for (int r = 0; r < num_states; r++)
        {
            if (rep[r] != r) continue;
            for (int a = 0; a < ALPHA; a++)
            {
                if (adj[r][a].empty()) continue;
                int t = adj[r][a][0];
                print_class(r, rep);        // from
                cout << " -- ";
                print_class(rep[t], rep);   // to
                cout << " -- " << char('a' + a) << endl; // edge
            }
        }
    }
    
    void minimize()
    {
        build_nonequivalent_list();
        mark_dependencies();
        int *rep = build_representatives();
        print_minimized(rep);
        delete[] rep;
    }
};


int main()
{
    // Test 1: first board example (accepting state: 4)
    DFA d1(5);
    d1.add_transition(0, 2, 'a');
    d1.add_transition(0, 1, 'b');
    d1.add_transition(1, 3, 'a');
    d1.add_transition(1, 1, 'b');
    d1.add_transition(2, 3, 'a');
    d1.add_transition(2, 2, 'b');
    d1.add_transition(3, 3, 'a');
    d1.add_transition(3, 4, 'b');
    d1.add_transition(4, 4, 'a');
    d1.add_transition(4, 4, 'b');
    d1.set_accepting(4);

    cout << "===== Test 1 =====" << endl;
    d1.minimize();

    // Test 2: second board example (accepting states: {4, 5})
    DFA d2(6);
    d2.add_transition(0, 1, 'a');
    d2.add_transition(0, 2, 'b');
    d2.add_transition(1, 4, 'a');
    d2.add_transition(1, 1, 'b');
    d2.add_transition(2, 4, 'a');
    d2.add_transition(2, 3, 'b');
    d2.add_transition(3, 4, 'a');
    d2.add_transition(3, 3, 'b');
    d2.add_transition(4, 4, 'a');
    d2.add_transition(4, 5, 'b');
    d2.add_transition(5, 5, 'a');
    d2.add_transition(5, 5, 'b');
    d2.set_accepting(4);
    d2.set_accepting(5);

    cout << endl << "===== Test 2 =====" << endl;
    d2.minimize();

    return 0;
}