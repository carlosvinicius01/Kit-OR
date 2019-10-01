#include "LinKerninghan.h"

Tour::Tour(vi &s, int V, double objValue)
{
    tour = s;
    tour.pop_back();
    cost = objValue;
    N = V;
    inv.assign(N + 1, 0);
    for (int i = 0; i < V; i++)
    {
        inv[tour[i]] = i;
    }
    reversed = false;
}

void Tour::print()
{
    for (int i = 0; i < tour.size(); i++)
    {
        std::cout << tour[i] << " ";
    }
    std::cout << "\n";
    // std::cout << tour[0] << " \n";
}

double Tour::getCost() { return cost; }
int Tour::inverse(int node) { return inv[node]; }
int Tour::next(int node)
{
    if (!reversed)
        return tour[(inv[node] + 1) % N];
    return tour[((inv[node] - 1) % N + N) % N];
}
int Tour::prev(int node)
{
    if (!reversed)
        return tour[((inv[node] - 1) % N + N) % N];
    return tour[(inv[node] + 1) % N];
}
int Tour::getN() { return N; }
vi Tour::getTour()
{
    return tour;
}

int breadth(int k)
{
    return (k <= 2) ? 5 : 1;
}

void Tour::flip(int a, int b)
{
    int segment_size = abs(inv[a] - inv[b]) + 1;

    if (!(!reversed && (inv[a] < inv[b]) || reversed && (inv[a] > inv[b])))
    {
        a = prev(a);
        b = next(b);
        segment_size = abs(inv[a] - inv[b]) + 1;
        reversed = !reversed;
    }

    if (segment_size > N / 2)
    {
        a = prev(a);
        b = next(b);
        reversed = !reversed;
        segment_size = N - segment_size;
    }
    for (int i = 0; i < segment_size / 2; i++)
    {
        swap(tour[inv[a]], tour[inv[b]]);
        int x = a, y = b;

        a = next(a);
        b = prev(b);

        swap(inv[x], inv[y]);
    }
}
bool Tour::sequence(int a, int b, int c)
{
    if (inv[a] < inv[b] && inv[b] < inv[c] || inv[c] < inv[b] && inv[b] < inv[a])
        return true;

    return false;
}

void lkStep(Tour &T, double **c, vector<vector<int>> &neighbourSet)
{
    double delta = 0;

    T.print();

    int base = 1, a;
    vector<bool> taken;
    taken.assign(T.getN() + 1, false);

    while ((a = findPromisingVertex(T, c, base, delta, taken, neighbourSet)) != -1)
    {
        delta += c[base][T.next(base)] - c[T.next(base)][a] + c[T.prev(a)][a] - c[T.prev(a)][base];
        cout << "\n"
             << T.next(base) << " " << a << " " << T.getCost() - delta << "\n";

        T.flip(T.next(base), T.prev(a));
        // T.print();

        vi s = T.getTour();
        // for (int i = 1; i <= T.getN(); i++)
        // {
        //     cout << T.inverse(i) << " ";
        // }
        std::cout << "\n";
        // printSolucao(s);
        cout << calcularValorObj(s, c) + c[s[s.size() - 1]][s[0]] << "\n";
    }
}

int findPromisingVertex(Tour &T, double **c, int base, double delta, vector<bool> &taken, vector<vector<int>> &neighbourSet)
{
    double A = delta + c[base][T.next(base)];

    pair<double, int> best_a = {-INF, -1};

    int k = 13;

    for (int i = 1; i <= k; i++)
    {
        int a = neighbourSet[T.next(base) - 1][i];
        pair<double, int> cost = {c[T.prev(a)][a] - c[T.next(base)][a], a};
        if (taken[a] == false && cost > best_a && T.prev(a) != base && T.prev(a) != T.next(base) && T.prev(a) != T.prev(base))
        {
            best_a = cost;
        }
    }

    if (best_a.second != -1)
    {
        taken[best_a.second] = true;
    }

    return best_a.second;
}

void step(Tour &T, double **c, int base, int level, float delta, vector<vector<int>> &neighbourSet, deque<pair<int, int>> &flipSequence, vector<bool> &taken)
{
    // cout << T.getCost() - delta << " " << level << "\n";

    // Create lk-ordering for next(base)
    vector<pair<pair<double, int>, bool>> lk_ordering;
    int lk_ordering_size = T.getN() - 3;
    lk_ordering.assign(lk_ordering_size, {{0, 0}, 0});

    for (int i = 0, a = T.next(T.next(T.next(base))); i < lk_ordering_size; i++)
    {
        // cout << a << "\n";
        double greedy_cost = c[T.prev(a)][a] - c[T.next(base)][a];
        lk_ordering[i] = {{-greedy_cost, a}, false};
        a = T.next(a);
    }

    // sort(lk_ordering.begin(), lk_ordering.end());

    for (int i = 0; i < lk_ordering.size(); i++)
    {
        // T.print();
        cout << T.next(base) << " " << lk_ordering[i].first.second << " " << -lk_ordering[i].first.first << "\n";
        // cout << T.getCost() - delta << "\n";
        // int a = lk_ordering[i].first.second;

        // if (taken[a] == false)
        // {
        //     taken[a] = true;

        //     double g = c[base][T.next(base)] - c[T.next(base)][a] + c[T.prev(a)][a] - c[T.prev(a)][base];
        //     // T.print();
        //     // cout << a << " " << base << "g: " << g << "\n";
        //     T.flip(T.next(base), T.prev(a));
        //     flipSequence.push_back({T.next(base), T.prev(a)});

        //     step(T, c, base, level + 1, delta + g, neighbourSet, flipSequence, taken);
        // }

        // if (delta > 0)
        // {
        //     return;
        // }
        // else
        // {
        //     if (!flipSequence.empty())
        //     {
        //         pair<int, int> fl = flipSequence.back();
        //         T.flip(fl.second, fl.first);
        //         flipSequence.pop_back();
        //     }
        // }
    }
}