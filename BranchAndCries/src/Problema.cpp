#include "Problema.h"

Problema::Problema(Data &d, double UB)
{
    this->UB = UB;
    bestInteger = UB;
    this->data = d;

    env1 = IloEnv();
    masterModel = IloModel(env1);
    lambda = IloNumVarArray(env1, data.getNItems(), 0.0, IloInfinity);
    masterRanges = IloRangeArray(env1);

    IloExpr sum(env1);
    for (int i = 0; i < data.getNItems(); i++)
    {
        sum += M * lambda[i];
        masterRanges.add(lambda[i] == 1);
    }

    masterModel.add(masterRanges);
    masterObj = IloObjective(env1);
    masterObj = IloMinimize(env1, sum);
    masterModel.add(masterObj);

    master = IloCplex(masterModel);

    pi = IloNumArray(env1, data.getNItems());

    lambdaItens = std::vector<std::vector<int>>(data.getNItems(), {1});

    master.setOut(env1.getNullStream());

    for (int i = 0; i < data.getNItems(); i++)
    {
        lambdaItens[i][0] = i;
    }

    try
    {
        master.solve();
    }

    catch (IloException &e)
    {
        std::cout << e;
    }
}

std::pair<int, int> Problema::solve(Node &node)
{
    IloEnv env2;
    IloModel pricingModel = IloModel(env2);

    IloExpr somaItems(env2);

    IloBoolVarArray x = IloBoolVarArray(env2, data.getNItems());

    for (int i = 0; i < data.getNItems(); i++)
    {
        somaItems += data.getItemWeight(i) * x[i];
    }

    pricingModel.add(somaItems <= data.getBinCapacity());
    IloObjective pricingObj = IloObjective(env2);
    pricingModel.add(pricingObj);

    IloCplex pricing = IloCplex(pricingModel);
    pricing.setOut(env2.getNullStream());

    std::vector<int> colunasProibidas;

    if (!node.is_root)
    {
        //restriçoes dos itens juntos
        for (auto &parAtual : node.juntos)
        {
            int i = parAtual.first;
            int j = parAtual.second;

            x[i].setLB(1.0);
            x[j].setLB(1.0);
        }

        // restriçoes dos itens separados
        for (auto &parAtual : node.separados)
        {
            int i = parAtual.first;
            int j = parAtual.second;

            IloConstraint cons = (x[i] + x[j] <= 1);

            pricingModel.add(cons);

            for (int k = data.getNItems(); k < lambdaItens.size(); k++)
            {
                for (int l = 0; l < lambdaItens[k].size() - 1; l++)
                {
                    if ((lambdaItens[k][l] == i && lambdaItens[k][l + 1] == j) || (lambdaItens[k][l] == j && lambdaItens[k][l + 1] == i))
                    {
                        lambda[k].setUB(0.0);
                        colunasProibidas.push_back(k);
                    }
                }
            }
        }
    }

    xPares = std::vector<std::vector<double>>(data.getNItems(), std::vector<double>(data.getNItems(), 0));

    try
    {
        master.solve();
    }
    catch (IloException &e)
    {
        std::cout << e;
    }

    node.LB = master.getObjValue();
    LB = master.getObjValue();

    bool gerouColuna = false;

    double lastPricingObj = std::numeric_limits<double>::infinity();
    std::vector<int> lastPricingSolution(1, 0);

    while (1)
    {
        master.getDuals(pi, masterRanges);

        IloExpr somaPricing(env2);

        for (int i = 0; i < data.getNItems(); i++)
        {
            somaPricing += -pi[i] * x[i];
        }

        pricingObj.setExpr(somaPricing);

        somaPricing.end();

        pricing.setOut(env2.getNullStream());

        try
        {
            pricing.solve();
        }

        catch (IloException &e)
        {
            std::cout << e;
        }

        if (pricing.getStatus() == IloAlgorithm::Infeasible)
        {
            std::cout << "A\n";

            if (!node.is_root)
            {

                for (auto &k : colunasProibidas)
                {
                    lambda[k].setUB(IloInfinity);
                }

                for (auto &parAtual : node.juntos)
                {
                    int i = parAtual.first;
                    int j = parAtual.second;

                    x[i].setLB(0.0);
                    x[j].setLB(0.0);
                }
            }

            pricingModel.end();
            env2.end();

            return {0, 0};
            break;
        }

        IloNumArray x_vals(env2, data.getNItems());
        pricing.getValues(x_vals, x);

        std::vector<int> itens;

        for (int i = 0; i < x_vals.getSize(); i++)
        {
            if (x_vals[i] > 1 - EPSILON)
            {
                itens.push_back(i);
            }
        }

        if (1 + pricing.getObjValue() < -EPSILON)
        {

            std::cout << "\n";

            lastPricingSolution = itens;
            lastPricingObj = pricing.getObjValue();
            gerouColuna = true;

            lambda.add(IloNumVar(masterObj(1) + masterRanges(x_vals), 0.0, IloInfinity));

            lambdaItens.push_back(itens);

            master.setOut(env1.getNullStream());

            try
            {
                master.solve();
            }
            catch (IloException &e)
            {
                std::cout << e;
            }

            std::cout << "lambda: ";
            for (int i = 0; i < lambda.getSize(); i++)
            {
                std::cout << master.getValue(lambda[i]) << " ";
            }
            std::cout << "\n";

            if (!node.is_root)
            {
                std::cout << "B\n";
                IloNumArray lambdaVals(env1, lambda.getSize());
                master.getValues(lambdaVals, lambda);

                for (int i = 0; i < data.getNItems(); i++)
                {
                    if (lambdaVals[i] > EPSILON)
                    {
                        if (!node.is_root)
                        {

                            for (auto &k : colunasProibidas)
                            {
                                lambda[k].setUB(IloInfinity);
                            }

                            for (auto &parAtual : node.juntos)
                            {
                                int i = parAtual.first;
                                int j = parAtual.second;

                                x[i].setLB(0.0);
                                x[j].setLB(0.0);
                            }
                        }

                        pricingModel.end();
                        env2.end();

                        return {0, 0};
                    }
                }
            }

            std::cout << "valor obj do mestre: " << master.getObjValue() << "\n";

            node.LB = master.getObjValue();
            LB = master.getObjValue();

            x_vals.end();
        }
        else
        {
            x_vals.end();

            break;
        }
    }

    if (std::ceil(master.getObjValue() - EPSILON) - bestInteger >= 0)
    {
        if (!node.is_root)
        {

            std::cout << "C\n";

            for (auto &k : colunasProibidas)
            {
                lambda[k].setUB(IloInfinity);
            }

            for (auto &parAtual : node.juntos)
            {
                int i = parAtual.first;
                int j = parAtual.second;

                x[i].setLB(0.0);
                x[j].setLB(0.0);
            }
        }

        pricingModel.end();
        env2.end();

        return {0, 0};
    }

    std::cout << master.getObjValue() << "\n";

    //branching rule

    IloNumArray lambdaVals(env1, lambda.getSize());
    master.getValues(lambdaVals, lambda);
    double deltaFrac = std::numeric_limits<double>::infinity();
    double tempDeltaFrac;

    std::vector<double> lambdaPares((data.getNItems() * data.getNItems() - data.getNItems()) / 2);

    for (int i = data.getNItems(); i < lambdaItens.size(); i++)
    {
        for (int j = 0; j < lambdaItens[i].size() - 1; j++)
        {
            xPares[lambdaItens[i][j]][lambdaItens[i][j + 1]] = (xPares[lambdaItens[i][j + 1]][lambdaItens[i][j]] += lambdaVals[i]);
        }
    }

    std::pair<int, int> branchingPair;

    for (int i = 0; i < data.getNItems(); i++)
    {
        for (int j = i + 1; j < data.getNItems(); j++)
        {
            std::pair<int, int> bp = {i, j};
            if (!node.is_root)
            {
                if ((std::find(node.juntos.begin(), node.juntos.end(), bp) != node.juntos.end()) || (std::find(node.separados.begin(), node.separados.end(), bp) != node.separados.end()))
                {
                    continue;
                }
                else
                {
                }
            }

            tempDeltaFrac = std::abs(0.5 - xPares[i][j]);
            if (tempDeltaFrac < deltaFrac)
            {
                branchingPair = {i, j};

                deltaFrac = tempDeltaFrac;
            }
        }
    }

    lambdaVals.end();

    if (std::abs(0.5 - deltaFrac) < EPSILON)
    {
        {
            if (master.getObjValue() < bestInteger)
            {
                bestInteger = master.getObjValue();
            }
        }

        if (!node.is_root)
        {

            std::cout << "D\n";
            for (auto &k : colunasProibidas)
            {
                lambda[k].setUB(IloInfinity);
            }

            for (auto &parAtual : node.juntos)
            {
                int i = parAtual.first;
                int j = parAtual.second;

                x[i].setLB(0.0);
                x[j].setLB(0.0);
            }
        }

        pricingModel.end();
        env2.end();

        return {0, 0};
    }

    pricingModel.end();
    env2.end();

    return branchingPair;
}
