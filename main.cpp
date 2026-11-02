#include <iostream>
using namespace std;
#define ALPHA 26


// a utility list used for as dynamic-sized array
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
    list<int> **adjecancy_list;
    list<int> **reversed_graph;
    bool **equivalent; // a table of size n x n to mark the equivalents of each state
    bool *is_accepting; // a frequency array to check if a state is an accepting state or a reqular one
    list<Pair> non_equivalent_pairs;

    DFA(int n) : num_states(n)
    {
        adjecancy_list = new list<int>* [n];
        reversed_graph = new list<int>* [n];
        equivalent = new bool*[n];
        is_accepting = new bool[n];
        for (int i = 0; i < n; i++)
        {
            adjecancy_list[i] = new list<int> [ALPHA];
            reversed_graph[i] = new list<int> [ALPHA];
            equivalent[i] = new bool[n];
            is_accepting[i] = false;
        }
    }

    ~DFA()
    {
        for (int i = 0; i < num_states; i++)
        {
            delete[] adjecancy_list[i];
            delete[] reversed_graph[i];
            delete[] equivalent[i];
        }
        delete[] adjecancy_list;
        delete[] reversed_graph;
        delete[] equivalent;
        delete[] is_accepting;
    }

    DFA(const DFA &) = delete;

    DFA& operator=(const DFA &) = delete;

    
    void add_transition(int from, int to, char sym)
    {
        adjecancy_list[from][sym - 'a'].push(to);
        reversed_graph[to][sym - 'a'].push(from);
    }

    
    void set_accepting(int state)
    {
        is_accepting[state] = true;
    }
    
    
    void build_nonequivalent_list() 
    { 
        /*
            fills the non_equivalent_pairs list 
            also builds the @equivalent matrix the determines if two states are equivalents
        */    
        
        // marking each pair of accepting and non-accepting state as not equivalent
        for (int p = 0; p < num_states; p++) 
            // stop at q <= p so we visit each pair only once as the matrix is symmetric
            for (int q = 0; q <= p; q++) {
                equivalent[p][q] = (is_accepting[p] == is_accepting[q]);
                equivalent[q][p] = equivalent[p][q];
                if (!equivalent[p][q])
                    non_equivalent_pairs.push(Pair(p,q));
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
                list<int> &pred_p = reversed_graph[p][a]; // list of states that go to @p on @a
                list<int> &pred_q = reversed_graph[q][a]; // list of states that go to @q on @a
    
                // constructing the combinations of p's predecessors and q's predessors 
                for (int i = 0; i < pred_p.size(); ++i)
                {
                    int r = pred_p[i];
                    for (int j = 0; j < pred_q.size(); ++j)
                    {
                        int s = pred_q[j];
                        
                        // reject pairs of the same state
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
        /*
        iterates over all states and assigns a representative to each state
        if a state is equivalent to a previous state, it takes the smallest equivalent state as its representative. otherwise, the state represents itself.

        for example if states 0,2,4 are equivalent and states 1,3 are equivalent,
        then the representatives will be {0,1,0,1,0}.
        */

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
        /*
        iterates over all states and prints the states that belong to the equivalence class represented by the given representative.
        a state belongs to the class if its representative matches the given r.

        for example if rep = {0,1,0,1,0} and r = 0,
        then it prints the equivalence class {0,2,4}.
        */

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
        /*
        prints the minimized DFA using the representative of each equivalence class. counts and prints the minimized states, accepting states, and transitions.

        for each representative, it prints its equivalence class.
        transitions are taken from the representative states because all states in the same equivalence class have equivalent transitions.
        */

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
                if (adjecancy_list[r][a].empty()) continue;
                int t = adjecancy_list[r][a][0];
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

    // Test 1: third board example (accepting state: 3)
    DFA d1(4);
    d1.add_transition(0, 1, 'a');
    d1.add_transition(0, 2, 'b');
    d1.add_transition(1, 3, 'a');
    d1.add_transition(1, 1, 'b');
    d1.add_transition(2, 3, 'a');
    d1.add_transition(2, 2, 'b');
    d1.add_transition(3, 3, 'a');
    d1.add_transition(3, 3, 'b');
    d1.set_accepting(3);

    cout << "\n===== Test 1 =====\n";
    d1.minimize();

    // Test 2: fourth board example (accepting state: {4})
    DFA d2(5);
    d2.add_transition(0, 2, 'a');
    d2.add_transition(0, 1, 'b');
    d2.add_transition(1, 3, 'a');
    d2.add_transition(1, 1, 'b');
    d2.add_transition(2, 3, 'a');
    d2.add_transition(2, 2, 'b');
    d2.add_transition(3, 3, 'a');
    d2.add_transition(3, 4, 'b');
    d2.add_transition(4, 4, 'a');
    d2.add_transition(4, 4, 'b');
    d2.set_accepting(4);

    cout << "\n===== Test 2 =====\n";
    d2.minimize();

    // Test 3: fifth board example (accepting states: {4, 5})
    DFA d3(6);
    d3.add_transition(0, 1, 'a');
    d3.add_transition(0, 2, 'b');
    d3.add_transition(1, 4, 'a');
    d3.add_transition(1, 1, 'b');
    d3.add_transition(2, 4, 'a');
    d3.add_transition(2, 3, 'b');
    d3.add_transition(3, 4, 'a');
    d3.add_transition(3, 3, 'b');
    d3.add_transition(4, 4, 'a');
    d3.add_transition(4, 5, 'b');
    d3.add_transition(5, 5, 'a');
    d3.add_transition(5, 5, 'b');
    d3.set_accepting(4);
    d3.set_accepting(5);

    cout << "\n===== Test 3 =====\n";
    d3.minimize();
    
    
    // Test 4: first board example (accepting states: {1, 2, 3})
    // replaced the alphabet symbol "letter" with "a" and the alphabet symbol "digit" with "b"
    // also coonverted it to 0-based instead of 1-based
    DFA d4(4);
    d4.add_transition(0, 1, 'a');
    d4.add_transition(1, 2, 'a');
    d4.add_transition(1, 3, 'b');
    d4.add_transition(2, 2, 'a');
    d4.add_transition(2, 3, 'b');
    d4.add_transition(3, 3, 'b');
    d4.set_accepting(1);
    d4.set_accepting(2);
    d4.set_accepting(3);
    
    cout << "\n===== Test 4 =====\n";
    d4.minimize();

    return 0;
}