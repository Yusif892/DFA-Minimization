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
    bool *is_accepting;

    DFA(int n) : num_states(n)
    {
        adj = new list<int>* [n];
        reversed_graph = new list<int>* [n];
        for (int i = 0; i < n; i++)
        {
            adj[i] = new list<int> [ALPHA];
            reversed_graph[i] = new list<int> [ALPHA];
        }
        is_accepting = new bool[n];
        for (int i = 0; i < n; i++) is_accepting[i] = false;
    }

    ~DFA()
    {
        for (int i = 0; i < num_states; i++)
        {
            delete[] adj[i];
            delete[] reversed_graph[i];
        }
        delete[] adj;
        delete[] reversed_graph;
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
};

void read_input(list<int> **&adj, list<int> **&reversed_graph, bool *&is_accepting, int &num_states)
{
    /* reads the input and fills the number of states, adjacency list, reversed graph and the flags of accepting states
    @param is_accepting is a list of all states each of which is flagged 1 if it is accepting and 0 otherwise
    */
    
    cout << "Enter the number of states: ";
    cin >> num_states;
    cout << endl;
    
    // initializing the adjacency lists
    adj = new list<int>* [num_states];
    reversed_graph = new list<int>* [num_states];
    for (int i = 0; i < num_states; i++)
    {
        adj[i] = new list<int> [26];
        reversed_graph[i] = new list<int> [26];
    }


    cout << "Enter the number of transitions: ";
    int transitions; cin >> transitions;
    cout << endl;

    cout << "Enter the transitions in this format: x y a\n which means there is a transition from state x to state y on symbol a"<<endl;
    for (int i = 0; i < transitions; i++)
    {
        int x, y; cin >> x >> y;
        char a; cin >> a;
        adj[x][a-'a'].push(y);
        reversed_graph[y][a-'a'].push(x);
    }

    cout << "Enter the number of accepting states: ";
    int accepting_states; cin >> accepting_states;
    cout << "Define the accepting states  (state numbers separated by spaces, ex. 2 4):"<<endl;
    
    // initializing the frequency array of all states to determine whether a state is accepted or not in O(1) 
    is_accepting = new bool[num_states ];
    for (int i = 0; i < num_states; i++) is_accepting[i] = false;

    // marking all the accepting states
    for (int i = 0; i < accepting_states; i++)
    {
        int state; cin >> state;
        is_accepting[state] = true;
    }

    
}


void build_nonequivalent_list(bool **&equivalent, const bool *is_accepting, list<Pair> &non_eq, const int num_states) 
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
                non_eq.push(Pair(i,j));
        }
}


void mark_dependencies(bool **&is_equivalent, list<Pair> &non_equivalent_pairs, list<int> **reverse_graph) 
{
    /*
        when two states p,q are found inequivalent, PLEASE change the @is_equivalent[p][q] to false
        iterate over the @non_equivalent_pairs, construct dependencies of each pair from the reversed graph
        and mark their states as non equivalents
    */

    for (int idx = 0; idx < non_equivalent_pairs.size(); ++idx)
    {
        Pair cur = non_equivalent_pairs[idx];
        int p = cur.p, q = cur.q;

        for (int a = 0; a < ALPHA; ++a)
        {
            list<int> &pred_p = reverse_graph[p][a]; // go to p by a
            list<int> &pred_q = reverse_graph[q][a]; // go to q b a

            for (int i = 0; i < pred_p.size(); ++i)
            {
                int r = pred_p[i];
                for (int j = 0; j < pred_q.size(); ++j)
                {
                    int s = pred_q[j];
                    if (r == s) continue;

                    if (is_equivalent[r][s])
                    {
                        is_equivalent[r][s] = false;
                        is_equivalent[s][r] = false;
                        non_equivalent_pairs.push(Pair(r, s));
                    }
                }
            }
        }
    }
}


int* build_representatives(bool **equivalent, int num_states)
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

// for example {1,2}
void print_class(int r, const int *rep, int num_states)
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

void print_minimized(list<int> **adj, const bool *is_accepting, const int *rep, int num_states)
{
    int m = 0;
    for (int i = 0; i < num_states; i++)
        if (rep[i] == i) m++;

    cout << "Number of states: " << m << endl;

    cout << "States: ";
    for (int i = 0; i < num_states; i++)
        if (rep[i] == i) { print_class(i, rep, num_states); cout << " "; }
    cout << endl;

    cout << "Accepting states: ";
    for (int i = 0; i < num_states; i++)
        if (rep[i] == i && is_accepting[i]) { print_class(i, rep, num_states); cout << " "; }
    cout << endl;

    cout << "Transitions (from -- to -- edge):" << endl;
    for (int r = 0; r < num_states; r++)
    {
        if (rep[r] != r) continue;
        for (int a = 0; a < ALPHA; a++)
        {
            if (adj[r][a].empty()) continue;
            int t = adj[r][a][0];
            print_class(r, rep, num_states);        // from
            cout << " -- ";
            print_class(rep[t], rep, num_states);   // to
            cout << " -- " << char('a' + a) << endl; // edge
        }
    }
}

void minimize(DFA &dfa)
{
    bool **equivalent = nullptr;
    list<Pair> non_equivalents;
    build_nonequivalent_list(equivalent, dfa.is_accepting, non_equivalents, dfa.num_states);
    mark_dependencies(equivalent, non_equivalents, dfa.reversed_graph);
    int *rep = build_representatives(equivalent, dfa.num_states);
    print_minimized(dfa.adj, dfa.is_accepting, rep, dfa.num_states);

    for (int i = 0; i < dfa.num_states; i++) delete[] equivalent[i];
    delete[] equivalent;
    delete[] rep;
}

int main()
{
    // ----- Test 1: first board example (accepting state: 4)
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
    minimize(d1);

    // ----- Test 2: second board example (accepting states: {4, 5})
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
    minimize(d2);

    return 0;
}