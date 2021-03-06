// A C++ program for Prim's Minimum
// Spanning Tree (MST) algorithm. The program is
// for adjacency matrix representation of the graph
#include "MinSpanningTree.h"
using namespace std;

void process(int vtx, std::vector<std::vector<dii>> &AdjList, vi &taken, priority_queue<dii> &pq, bool skipFirstNode, std::vector<std::vector<int>> &rankedNodes)
{ // so, we use -ve sign to reverse the sort order
    taken[vtx] = 1;

    int max_iterations = std::min((int)AdjList[vtx].size(), 55);

    for (int j = 0; j < max_iterations; j++)
    {
        dii v = AdjList[vtx][rankedNodes[vtx][j]];
        if (!taken[v.second.first])
            pq.push({-v.first, {-v.second.first, -v.second.second}});
    }
} // sort by (inc) weight then by (inc) id

double MST(int V, std::vector<std::vector<dii>> &AdjList, vi &taken, vi &parent, vii &s, bool skipFirstNode, std::vector<std::vector<int>> &rankedNodes, std::vector<int> &v)
{
    priority_queue<dii> pq;

    v.assign(V, -2);

    // inside int main()---assume the graph is stored in AdjList, pq is empty
    taken.assign(V, 0); // no vertex is taken at the beginning
    process(0, AdjList, taken, pq, false, rankedNodes); // take vertex 0 and process all edges incident to vertex 0
    double mst_cost = 0;
    int u, a, w, k = 0;
    while (!pq.empty())
    { // repeat until V vertices (E=V-1 edges) are taken
        dii front = pq.top();

        pq.pop();
        u = -front.second.first, a = -front.second.second, w = -front.first; // negate the id and weight again

        if (!taken[u])
        {                                                                              // we have not connected this vertex yet
            mst_cost += w, process(u, AdjList, taken, pq, skipFirstNode, rankedNodes); // take u, process all edges incident to u
            parent[u] = a;
            v[u]++;
            v[a]++;
            s[k++] = {a, u};
        }
    } // each edge is in pq only once!

    return mst_cost;
}

double MS1T(int V, std::vector<std::vector<dii>> &AdjList, vi &taken, vi &parent, vii &s, std::vector<std::vector<int>> &rankedNodes, std::vector<int> &v)
{
    double cost = MST(V, AdjList, taken, parent, s, false, rankedNodes, v);

    dii secondNeighbour = {-std::numeric_limits<double>::infinity(), {V, V}};
    for (int i = 0; i < V; i++)
    {
        if(v[i] == -1)
        secondNeighbour = std::max(AdjList[rankedNodes[i][2]][i], secondNeighbour);
    }

    s[V - 1] = {secondNeighbour.second.first, secondNeighbour.second.second};
    v[secondNeighbour.second.first]++;
    v[secondNeighbour.second.second]++;

    cost+=secondNeighbour.first;

    return cost;
}

void Ascent(int V, std::vector<std::vector<dii>> &AdjList, vi &taken, vi &parent, vii &edges, std::vector<std::vector<int>> &rankedNodes)
{
    // int k = 0;
    double W = -std::numeric_limits<double>::infinity(), prev_W, bestW;
    std::vector<double> pi, best_pi;
    vi v, last_v;
    int period = V / 2, iter = 0;
    bool firstPeriod = true;

    std::vector<std::vector<dii>> AdjListCopy = AdjList;

    pi.assign(V, 0);
    best_pi.assign(V, 0);

    // v.assign(V, 2);
    last_v.assign(V, 0);

    double t0 = 1;

    while (1)
    {
        // edges.assign(V, {-3, -3});

        double T = MS1T(V, AdjList, taken, parent, edges, rankedNodes, v);
        double PI_SUM = 0;
        for (int i = 0; i < V; i++)
        {
            PI_SUM += pi[i];
        }
        T -= 2 * PI_SUM;

        prev_W = W;

        //  std::cout << T << ", " << t0 << "\n";

        bool isFeasible = true;
        for (int i = 0; i < V; i++)
        {
            if (v[i] != 0)
            {
                isFeasible = false;
                break;
            }
        }

        if (isFeasible)
        {
            break;
        }

        double t = t0;

        for (int i = 0; i < V; i++)
        {
            pi[i] += (7 * t * v[i] + 3 * t * last_v[i])/10.0;
        }

        for (int i = 0; i < V - 1; i++)
        {
            last_v[i] = v[i];
            for (int j = i + 1; j < V; j++)
            {
                AdjList[j][i].first = (AdjList[i][j].first = AdjListCopy[i][j].first + pi[i] + pi[j]);
            }
        }

        if (T > W + EPSILON)
        {
            W = T;
            if (firstPeriod)
                t0 *= 2;
            if (iter == period)
                period *= 2;
        }
        else if (firstPeriod && iter > V / 4)
        {
            firstPeriod = false;
            iter = 0;
            t0 = 3 * t0 / 4.0;
        }

        iter++;

        if (iter > period)
        {
            iter = 0;
            t0 /= 2.0;
            period /= 2;
        }

        if (period == 0 || t0 < EPSILON)
            break;
    }

    edges.assign(V - 1, {0, 0});

    // for (int i = 0; i < V; i++)
    // {
    //     for (int j = 0; j < V; j++)
    //     {
    //         std::cout << AdjList[i][j].first << " ";
    //     }
    //     std::cout << "\n";
    // }

    MST(V, AdjList, taken, parent, edges, false, rankedNodes, v);

    // for (int i = 0; i < V; i++)
    // {
    // std::cout << v[i] << "\n";
    // }
}
