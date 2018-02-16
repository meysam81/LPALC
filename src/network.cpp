#include "network.h"

Network::Network() : numberOfNodes(0)
{
}

bool Network::init(string inputPath, int numberOfRealClusters)
{
    try
    {
        QFile inputFile(QString::fromStdString(inputPath));

        if (!inputFile.open(QFile::ReadOnly)) // try to open the file
            return false;

        this->numberOfRealClusters = numberOfRealClusters;



        // first loop to get the number of nodes
        while (!inputFile.atEnd()) // read until EOF
        {
            QString line = inputFile.readLine(); // read line by line
            line = line.replace("\r\n", "\0"); // remove the endl
            line = line.replace("\n", "\0"); // remove the endl

            QStringList list = line.split('\t'); // get two nodes

            int v1 = list.at(0).toInt(),
                    v2 = list.at(1).toInt();

            int value = (v2 > v1 ? v2 : v1);

            if (value > numberOfNodes)
                numberOfNodes = value;
        }


        // now get the edges
        inputFile.seek(0); // put the cursor at the beginning of the file


        // node labels start from zero so we should initializr from 1 to n
        edges = new SparseMatrix<int>(numberOfNodes);

        nodes = new vector<Node>(numberOfNodes + 1);
        for (int i = 1; i <= numberOfNodes; ++i)
        {
            nodes->at(i).label = i;
            nodes->at(i).degree = 0;
            nodes->at(i).newLabel = i;
        }


        // second loop to get the edges
        while (!inputFile.atEnd()) // get number of nodes
        {
            QString line = inputFile.readLine();
            line = line.replace("\r\n", "\0");
            line = line.replace("\n", "\0");


            QStringList list = line.split('\t');


            int v1 = list.at(0).toInt(),
                    v2 = list.at(1).toInt();


            // indirected graph; having edge in both direction
            edges->set(v1, v2, 1);
            edges->set(v2, v1, 1);

            // add the corresponding degree
            nodes->at(v1).degree++;
            nodes->at(v2).degree++;

        }



        // close the reading file
        inputFile.close();

        return true;
    }
    catch (exception ex)
    {
        cerr << ex.what() << endl;
        return false;
    }

}

bool Network::mainFunc()
{
    int t = 1;
    while (NMI() >= exp(1)
           && (t <= numberOfNodes))
    {
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(nodes->begin() + 1, nodes->end(), default_random_engine(seed));


        for (auto i = nodes->begin() + 1; i != nodes->end(); ++i)
        {
            vector<int> *tmpNeighbors;
            *tmpNeighbors = edges->getNeighbors(i->degree);


            vector<Node> neighbors(tmpNeighbors->size() + 1);


            /* the calculations are based on the labels or as we call it 'newLabel'
             * not the node number itself which is just a distinction of nodes
             */
            for (int i = 1; i <= tmpNeighbors->size(); i++)
            {
                int tmp = tmpNeighbors->at(i - 1);
                neighbors.at(i).label = tmp;
                neighbors.at(i).degree = nodes->at(i).degree;
                neighbors.at(i).newLabel = nodes->at(i).newLabel;
            }



            map<Node, int> neighborFreq;
            foreach (auto j, neighbors)
                if (neighborFreq.count(j) == 0)
                    neighborFreq.insert(make_pair(j, std::count(neighbors.begin(),
                                                                neighbors.end(),
                                                                j)));


            map<Node, int>::iterator iter = neighborFreq.begin();

            int numberOfMax = 1;
            int maxFreq = iter->second;
            iter++;

            for (; iter != neighborFreq.end(); iter++)
            {
                if (iter->second > maxFreq)
                {
                    numberOfMax = 1;
                    maxFreq = iter->second;
                }
                else if (iter->second == maxFreq)
                    numberOfMax++;
            }

            /* difference between LPA and LPACL happens here cause it looks
             * for cycles in case of multiple labels having the same number
             * of repetition in a node's neighbors list
             */
            if (numberOfMax > 1)
            {
                vector<int> shortestCycle;
                computeShortestCycle(i->degree, neighbors, shortestCycle);

                if (shortestCycle.size() != 0)
                {
                    // CHANGE

                }
                else
                {
                    // CHANGE

                }
            }
            else // only one type of labels are in the node's neighbors list
            {
                // CHANGE
            }

        }
        t++;
    }
}

int Network::NMI()
{
    // CHANGE
}





//====================== reference: geeksforgeeks.org ========================
void Network::computeShortestCycle(const int startingPoint,
                                   const vector<Node> &neighbors,
                                   vector<int> &shortestCycle)
{
    int E = edges->numberOfRowElement(startingPoint);
    auto minCycle = INT64_MAX;
    for (int i = 0 ; i < E  ; i++)
    {
        /* get current edge vertices which we currently
         * remove from graph and then find shortest path
         * between these two vertex using Dijkstra’s
         * shortest path algorithm.
         */

        edges->removeEdge(startingPoint, neighbors.at(i).label);

        // minimum distance between these two vertices
        int distance;
        vector<int> tmpShrotestCycle;
        if ((distance = shortestPath(startingPoint, neighbors.at(i).label,
                                     tmpShrotestCycle)) < minCycle)
        {
            minCycle = distance;
            shortestCycle = tmpShrotestCycle;
        }

        // to make a cycle we have to add weight of
        // currently removed edge if this is the shortest
        // cycle then update minCycle

        //  add current edge back to the graph
        edges->set(startingPoint, neighbors.at(i).label, 1);
    }
}
int Network::shortestPath(int u, int v, vector<int> &tmpShortestCycle)
{

}

bool operator <(const Node& node1, const Node& node2)
{
    return node1.degree < node2.degree;
}
bool operator ==(Node& node1, const Node& node2)
{
    return node1.degree == node2.degree;
}
