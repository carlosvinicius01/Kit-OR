#include "Data.h"
#include "Problema.h"
#include "Node.h"
#include <stdio.h>
#include <iostream>
#include <list>
#include <ilcplex/ilocplex.h>
#include <chrono>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage:\n./bin instance\n");
        return 0;
    }

    Data data;
    data.readData(argv[1]);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    Problema p(data, std::numeric_limits<double>::infinity());

    Node raiz;
    raiz.is_root = true;
    raiz.separados = {};
    raiz.juntos = {};

    std::pair<int, int> branchingPair = p.solve(raiz);

    Node n1, n2;
    n1.juntos.push_back(branchingPair);
    n1.tipo_branch = true;
    n1.is_root = false;
    n2.separados.push_back(branchingPair);
    n2.tipo_branch = false;
    n2.is_root = false;

    std::list<Node> tree = {n1, n2};
    auto node_it = tree.begin();

    int k = 0;

    std::cout << "\n"
              << std::string(argv[1]) << "\n";
    // while(k--)
    while (!tree.empty())
    {
        // std::cout << "\n\n";

        // double maiorLB = -std::numeric_limits<double>::infinity();
        // for (auto it = tree.begin(); it != tree.end(); it++)
        // {
        //     if (it->LB > maiorLB)
        //     {
        //         maiorLB = it->LB;
        //         node_it = it;
        //     }
        // }

        node_it = tree.end();
        node_it--;

        branchingPair = p.solve(*node_it);

        if (!(branchingPair.first == 0 && branchingPair.second == 0))
        {
            Node nj, ns;
            ns = *node_it;
            nj = *node_it;

            ns.is_root = false;
            nj.is_root = false;

            nj.juntos.push_back(branchingPair);
            nj.tipo_branch = true;

            ns.separados.push_back(branchingPair);
            ns.tipo_branch = false;

            tree.push_back(ns);
            tree.push_back(nj);
        }

        // std::cout << "Num nodes: " << tree.size() << "\n";
        // std::cout << "best integer: " << p.bestInteger << "\n";

        tree.erase(node_it);

        // std::cout << "\n\n";
    }

    std::cout << "Bins usados: " << p.bestInteger << "\n";

    end = std::chrono::system_clock::now();

    int elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Elapsed time (s): " << elapsed_seconds / 1000.0 << "\n\n";

    return 0;
}
