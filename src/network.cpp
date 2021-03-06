#include "network.h"

Network::Network() : numberOfNodes(0)
{
}

bool Network::init(const string inputPath)
{
    try
    {
        QFile inputFile(QString::fromStdString(inputPath));

        if (!inputFile.open(QFile::ReadOnly)) // try to open the file
            return false;

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
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    srand(seed);
    int e = ((double) rand() / RAND_MAX);
    while (NMI() >= e
           && (t <= numberOfNodes))
    {
        seed = chrono::system_clock::now().time_since_epoch().count();
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

                neighbors.at(i).degree =
                        nodes->at(tmpNeighbors->at(i - 1)).degree;

                neighbors.at(i).newLabel =
                        nodes->at(tmpNeighbors->at(i - 1)).newLabel;
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

            vector<int> maxFreqLabels;
            maxFreqLabels.push_back(iter->first.label);

            for (; iter != neighborFreq.end(); iter++)
            {
                if (iter->second > maxFreq)
                {
                    numberOfMax = 1;
                    maxFreq = iter->second;
                    maxFreqLabels.clear();
                    maxFreqLabels.push_back(iter->first.label);
                }
                else if (iter->second == maxFreq)
                {
                    numberOfMax++;
                    maxFreqLabels.push_back(iter->first.label);
                }
            }

            /* difference between LPA and LPACL happens here cause it looks
             * for cycles in case of multiple labels having the same number
             * of repetition in a node's neighbors list
             */
            if (numberOfMax > 1)
            {
                vector<int> shortestCycle;
                computeShortestCycle(i->degree, neighbors, shortestCycle);

                if (shortestCycle.size() > 1)
                {
                    i->newLabel = shortestCycle.at(1);

                }
                else
                {
                    // choosing a random label having the maximum frequency
                    i->newLabel = nodes->at(
                            maxFreqLabels.
                            at(rand() % maxFreqLabels.size() + 1)).label;

                }
            }
            else // only one type of labels are in the node's neighbors list
            {
                i->newLabel = maxFreqLabels.at(0);
            }

        }
        t++;


    }
}

int Network::NMI()
{
    return 1;
}

//================= reference: geeksforgeeks.org (refined) ===================
void Network::computeShortestCycle(const int startingPoint,
                                   const vector<Node> &neighbors,
                                   vector<int> &shortestCycle)
{
    int E = edges->numberOfRowElement(startingPoint);
    auto minCycle = INT32_MAX;
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
                                     tmpShrotestCycle,
                                     neighbors)) < minCycle)
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
int Network::shortestPath(const int u, const int v,
                          vector<int> &tmpShortestCycle,
                          const vector<Node> &neighbors)
{
    /* find shortest path from source to destination using Dijkstra’s
     * shortest path algorithm [Time complexity O(E.logV)]
     */


    // Create a set to store vertices that are being processed
    set< pair<int, int> > nextEdge;

    // Create a vector for distances and initialize all
    // distances as infinite
    vector<int> dist(neighbors.size(), INT16_MAX);

    // Insert source itself in Set and initialize its distance as 0
    nextEdge.insert(make_pair(0, u));
    dist[u] = 0;

    /* Looping till all uhortest vistance are finalized
     * then setds will become empty
     */


    // the cycle starts from the source node
    tmpShortestCycle.push_back(u);


    while (!nextEdge.empty())
    {
        /* The first vertex in Set is the minimum distance
         * vertex, extract it from set
         */
        pair<int, int> tmp = *(nextEdge.begin());
        nextEdge.erase(nextEdge.begin());

        /* vertex label is stored in second of pair (it has to be done this
         * way to keep the vertices sorted distance (distance must be first
         * item in pair)
         */
        int u = tmp.second;

        // 'i' is used to get all adjacent vertices of vertex 'u'
        for (auto i = neighbors.begin() + 1; i != neighbors.end(); ++i)
        {
            // Get vertex label and weight of current adjacent of 'u'
            int v = (*i).label;
            int weight = edges->get(u, v);

            // If there is a shorter path to v through u
            if (dist[v] > dist[u] + weight)
            {
                /* If distance of v is not INT16_MAX then it must be in
                 * our set, so removing it and inserting again with
                 * updated less distance.
                 * Note : We extract only those vertices from Set
                 * for which distance is finalized. So for them,
                 * we would never reach here
                 */
                if (dist[v] != INT16_MAX)
                    nextEdge.erase(nextEdge.find(make_pair(dist[v], v)));

                tmpShortestCycle.push_back(v);

                // Updating distance of v
                dist[v] = dist[u] + weight;
                nextEdge.insert(make_pair(dist[v], v));
            }
        }
    }

    // return shortest path from current Source to destination
    return dist[v] ;
}

bool operator <(const Node& node1, const Node& node2)
{
    return node1.degree < node2.degree;
}
bool operator ==(Node& node1, const Node& node2)
{
    return node1.degree == node2.degree;
}
