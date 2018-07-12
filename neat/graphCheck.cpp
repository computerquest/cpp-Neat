#include <iostream> 
#include <fstream> 
#include <string> 
#include <vector> 
#include <map> 
#include <set> 
#include <algorithm>
#include "Network.h"
#include "Node.h"
#include "graphCheck.h"
#include <mutex> //for whatever reason this contains the function keyword
using namespace std;

/*
want to find which nodes are the same between the two networks
I am going to start with the least degree node and compare those between the networks
if there isn't a match for the node then all the nodes that recieve input from it can't be the same
will need to check mutation to make sure not creating similar node through mutation
*/
map<int, int> calcBetween(Network& aNet, Network& bNet) {
	vector<vector<Node*>> aNode;
	vector<vector<Node*>> bNode;
	map<int, int> ans;
	for (int i = 0; i < aNet.input.size(); i++) {
		int id = aNet.input[i]->id;
		ans[id] = id;
	}
	/*
	could optimize so that it goes from node to node crawling through the sends using prefound values
	*/


	function<int(Node&)> trace;

	trace = [&trace](Node& n) -> int {
		int leastDist = 0;

		if (n.recieve.size() == 0) {
			return 0;
		}

		for (int i = 0; i < n.recieve.size(); i++) {
			int val = trace(*(n.recieve[i]->nodeFrom));
			if (leastDist < val) {
				leastDist = val;
			}
		}

		return leastDist + 1;
	};

	//creates the layers
	for (int i = 0; i < aNet.nodeList.size(); i++) {
		int layer = trace(aNet.nodeList[i]);

		if (layer > ((int)aNode.size() - 1)) {
			int prelim = ((int)aNode.size() - 1);
			for (int a = 0; a < layer - prelim; a++) {
				aNode.push_back(vector<Node*>());
			}
		}

		aNode[layer].push_back(&aNet.nodeList[i]);
	}
	for (int i = 0; i < bNet.nodeList.size(); i++) {
		int layer = trace(bNet.nodeList[i]);

		if (layer >((int)bNode.size() - 1)) {
			int prelim = ((int)bNode.size() - 1);
			for (int a = 0; a < layer - prelim; a++) {
				bNode.push_back(vector<Node*>());
			}
		}

		bNode[layer].push_back(&bNet.nodeList[i]);
	}

	map<int, pair<vector<int>, vector<int>>> inputGraph;
	map<int, pair<vector<int>, vector<int>>> inputGraphB;

	//adjusted for input layer and output layer
	for (int i = 0; i < aNet.nodeList.size(); i++) {
		vector<int> input;
		vector<int> output;

		for (int b = 0; b < aNet.nodeList.size(); b++) {
			input.push_back(0);
			output.push_back(0);
		}

		Node& n = aNet.nodeList[i];

		for (int b = 0; b < n.recieve.size(); b++) {
			input[n.recieve[b]->nodeFrom->id] = 1;
		}

		for (int b = 0; b < n.send.size(); b++) {
			output[n.send[b].nodeTo->id] = 1;
		}

		inputGraph[n.id] = pair<vector<int>, vector<int>>(input, output);
	}
	for (int i = 0; i < bNet.nodeList.size(); i++) {
		vector<int> input;
		vector<int> output;

		for (int b = 0; b < bNet.nodeList.size(); b++) {
			input.push_back(0);
			output.push_back(0);
		}

		Node& n = bNet.nodeList[i];

		for (int b = 0; b < n.recieve.size(); b++) {
			input[n.recieve[b]->nodeFrom->id] = 1;
		}

		for (int b = 0; b < n.send.size(); b++) {
			output[n.send[b].nodeTo->id] = 1;
		}

		inputGraphB[n.id] = pair<vector<int>, vector<int>>(input, output);
	}

	vector<vector<Node*>>* smaller;
	map<int, pair<vector<int>, vector<int>>>* sg;
	vector<vector<Node*>>* bigger;
	map<int, pair<vector<int>, vector<int>>>* bg;

	if (aNode.size() > bNode.size()) {
		smaller = &bNode;
		sg = &inputGraphB;
		bigger = &aNode;
		bg = &inputGraph;
	}
	else {
		smaller = &aNode;
		sg = &inputGraph;
		bigger = &bNode;
		bg = &inputGraphB;
	}

	auto createGraph = [](vector<vector<int>>& graph, Node& startNode, vector<vector<Node*>>* smaller, map<int, pair<vector<int>, vector<int>>>* sg) {
		//gets list of included nodes
		vector<int> included;
		vector<Node*> currentNodes;
		currentNodes.push_back(&startNode);

		while (true) {
			for (int b = 0; b < currentNodes.size(); b++) {
				included.push_back(currentNodes[b]->id);
			}

			vector<Node*> rep;
			for (int b = 0; b < currentNodes.size(); b++) {
				for (int c = 0; c < currentNodes[b]->recieve.size(); c++) {
					rep.push_back(currentNodes[b]->recieve[c]->nodeFrom);
				}
			}

			currentNodes = rep;

			if (currentNodes.size() == 0) {
				break;
			}
		}

		sort(included.begin(), included.end());
		included.erase(unique(included.begin(), included.end()), included.end());

		for (int b = 0; b < included.size(); b++) {
			auto current = (*sg)[included[b]];

			vector<int> f;
			vector<int> s;
			//erase is considered inefficient because it remakes the vector every time
			for (int c = included.size() - 1; c >= 0; c--) {
				s.push_back(current.second[included[c]]);
				f.push_back(current.first[included[c]]);
			}

			current.first = f;
			current.second = s;

			vector<int> connections;
			if (included[b] != startNode.id) {
				for (int c = 0; c < current.first.size(); c++) {
					if (current.first[c] == 1 || current.second[c] == 1) {
						connections.push_back(1);
					}
					else {
						connections.push_back(0);
					}
				}

				graph.push_back(connections);
			}
			else {
				graph.push_back(current.first);
			}
		}
	};

	//could optimize so only create graphs 1
	for (int i = 1; i < smaller->size() - 1; i++) {
		for (int a = 0; a < (*smaller)[i].size(); a++) {
			vector <vector<int>> graph;

			createGraph(graph, *(*smaller)[i][a], smaller, sg);

			//need to check to make sure not comparing nodes that already have been assigned
			for (int b = 0; b < (*bigger)[i].size(); b++) {
				vector<vector<int>> graphb;

				createGraph(graphb, *(*bigger)[i][b], bigger, bg);

				if (calc(graph, graphb)) {
					ans[(*smaller)[i][a]->id] = (*bigger)[i][b]->id;
					break;
				}
			}

			//need to make sure not checking the later nodes that are connected to earlier nodes that do not have a comporable node in the other network
		}
	}

	return ans;
}

bool calc(vector<vector<int>> rgraphA, vector<vector<int>> rgraphB)
{
	//Process Graph A 
	int i, j, k, q, N1, N2, nA;
	nA = rgraphA.size();

	//Initial sorting 
	vector<vector<int> > distanceA;
	vector<vector<int> > neighborA;
	for (i = 0; i < rgraphA.size(); i++)
	{
		vector<int> rowdistanceA, rowneighborA;
		for (j = 0; j < rgraphA[i].size(); j++)
		{
			if (rgraphA[i][j] == 1) rowneighborA.push_back(j);
			vector<int> indexdA;
			for (k = 0; k < rgraphA.size(); k++) indexdA.push_back(k);
			indexdA[0] = i; indexdA[i] = 0;
			vector<vector<int> > dgraphA = reindex(rgraphA, indexdA);
			vector<vector<int> > dpathA = dijkstra(dgraphA);
			int ddA = -1;
			for (k = 0; k < dpathA.size(); k++)
				if (inv(indexdA)[dpathA[k][dpathA[k].size() - 1]] == j)
				{
					ddA = dpathA[k].size() - 1; break;
				}
			rowdistanceA.push_back(ddA);
		}
		distanceA.push_back(rowdistanceA);
		neighborA.push_back(rowneighborA);
	}

	vector<vector<int> > sequenceA;
	for (i = 0; i < rgraphA.size(); i++)
	{
		vector<int> mutualdistanceA;
		for (j = 0; j < neighborA[i].size() - 1; j++)
			for (k = j + 1; k < neighborA[i].size(); k++)
				mutualdistanceA.push_back(distanceA[neighborA[i][j]][neighborA[i][k]]);
		sort(mutualdistanceA.begin(), mutualdistanceA.end());
		vector<int> tmutualA;
		for (k = 0; k < nA*nA - 1 - mutualdistanceA.size(); k++) tmutualA.push_back(0);
		for (k = 0; k < mutualdistanceA.size(); k++) tmutualA.push_back(mutualdistanceA[k]);
		sequenceA.push_back(tmutualA);
	}

	map<vector<int>, int> sorterA;
	for (i = 0; i < sequenceA.size(); i++) sorterA[sequenceA[i]] = i;
	vector<int> mainindexA;
	map<vector<int>, int>::iterator sitA = sorterA.begin();
	while (sitA != sorterA.end())
	{
		vector<int>vsitA = sitA->first;
		for (i = 0; i < sequenceA.size(); i++)
			if (sequenceA[i] == vsitA) mainindexA.push_back(i);
		sitA++;
	}

	vector< vector<int> > graphA = reindex(rgraphA, mainindexA);

	//Compute degree sequence 
	vector<int> degA;
	int sumA;
	for (i = 0; i < nA; i++)
	{
		sumA = 0;
		for (j = 0; j < nA; j++)
		{
			if (graphA[i][j] == 1) sumA++;
		}
		degA.push_back(sumA);
	}
	vector<int> sorted_degA = degA;
	sort(sorted_degA.begin(), sorted_degA.end());

	//Pair graphs 
	map<vector<int>, set<int> > PA;
	map<vector<int>, int> dA;
	map<vector<int>, int> ntA;
	map<vector<int>, int> eA;

	for (N1 = 0; N1 < graphA.size(); N1++)
		for (N2 = 0; N2 < graphA[N1].size(); N2++)
		{
			int signA = -1;
			vector<vector<int> > tgraphA = graphA;
			if (graphA[N1][N2] != 0) { tgraphA[N1][N2] = 0; tgraphA[N2][N1] = 0; signA = 1; }
			//Compute shortest paths from vertex 1  
			vector<vector<int> > shortest_path1A;
			vector<int> index1A; for (q = 0; q < nA; q++) index1A.push_back(q);
			index1A[0] = N1; index1A[N1] = 0;
			vector< vector<int> > temp_graph1A = reindex(tgraphA, index1A);
			vector< vector<int> > p1A = dijkstra(temp_graph1A);
			for (i = 0; i < p1A.size(); i++)
			{
				vector<int> tpath1A;
				for (j = 0; j < p1A[i].size(); j++) { tpath1A.push_back(inv(index1A)[p1A[i][j]]); }
				shortest_path1A.push_back(tpath1A);
			}
			//Compute shortest paths from vertex 2  
			vector<vector<int> > shortest_path2A;
			vector<int> index2A; for (q = 0; q < nA; q++) index2A.push_back(q);
			index2A[0] = N2; index2A[N2] = 0;
			vector< vector<int> > temp_graph2A = reindex(tgraphA, index2A);
			vector< vector<int> > p2A = dijkstra(temp_graph2A);
			for (i = 0; i < p2A.size(); i++)
			{
				vector<int> tpath2A;
				for (j = 0; j < p2A[i].size(); j++) { tpath2A.push_back(inv(index2A)[p2A[i][j]]); }
				shortest_path2A.push_back(tpath2A);
			}
			//Compute distance between vertex 1 and vertex 2 
			int DA = 0;
			for (i = 0; i < shortest_path1A.size(); i++)
				if (shortest_path1A[i][shortest_path1A[i].size() - 1] == N2)
				{
					DA = shortest_path1A[i].size() - 1; break;
				}
			//Compute shortest paths between vertex1 and vertex 2 
			vector<vector<int> > shortest_path12A;
			for (i = 0; i < shortest_path1A.size(); i++)
				for (j = 0; j < shortest_path2A.size(); j++)
				{
					if (shortest_path1A[i][shortest_path1A[i].size() - 1] ==
						shortest_path2A[j][shortest_path2A[j].size() - 1])
					{
						vector<int> temppathA = shortest_path1A[i];
						for (k = shortest_path2A[j].size() - 2; k >= 0; k--)
							temppathA.push_back(shortest_path2A[j][k]);
						if (temppathA.size() - 1 == DA)
							shortest_path12A.push_back(temppathA);
					}
				}
			//Pair graph for vertex 1 and vertex 2 
			bool checkA = false;
			set<int> SA;
			for (i = 0; i < shortest_path12A.size(); i++)
			{
				if (shortest_path12A[i][0] == N1 &&
					shortest_path12A[i][shortest_path12A[i].size() - 1] == N2)
				{
					checkA = true;
					for (j = 0; j < shortest_path12A[i].size(); j++)
						SA.insert(shortest_path12A[i][j]);
				}
			}
			vector<int> VA; VA.push_back(N1); VA.push_back(N2);
			PA[VA] = SA;
			//Distance between vertex 1 and vertex 2 
			if (checkA) dA[VA] = signA * DA;
			else dA[VA] = signA;
			//Count number of vertices in pair graph 
			ntA[VA] = SA.size();
			//Count number of edges in pair graph 
			int countA = 0;
			for (i = 0; i < nA; i++)
				for (j = i + 1; j < nA; j++)
				{
					bool findpairA = false;
					set<int>::iterator i1A, i2A;
					i1A = SA.find(i); i2A = SA.find(j);
					if (i1A != SA.end() && i2A != SA.end()) findpairA = true;
					if (findpairA && graphA[i][j] != 0) countA++;
				}
			eA[VA] = countA;
		}

	//Make frequency table (sign frequency vectors) 
	map<vector<int>, int>::iterator itA;
	map<vector<int>, vector<int> > frequencyA;
	vector<int> dummyA; for (i = 0; i < nA; i++) dummyA.push_back(0);
	for (i = 0; i < nA; i++)
	{
		for (j = 0; j < nA; j++)
		{
			vector<int> vecA;
			vector<int> indA; indA.push_back(i); indA.push_back(j);
			itA = dA.find(indA);
			vecA.push_back(itA->second);
			itA = ntA.find(indA);
			vecA.push_back(itA->second);
			itA = eA.find(indA);
			vecA.push_back(itA->second);
			frequencyA[vecA] = dummyA;
		}
	}

	map<vector<int>, vector<int> >::iterator ittA = frequencyA.begin();
	while (ittA != frequencyA.end())
	{
		for (i = 0; i < nA; i++)
		{
			int fA = 0;
			for (j = 0; j < nA; j++)
			{
				vector<int> vecA;
				vector<int> indA; indA.push_back(i); indA.push_back(j);
				itA = dA.find(indA);
				vecA.push_back(itA->second);
				itA = ntA.find(indA);
				vecA.push_back(itA->second);
				itA = eA.find(indA);
				vecA.push_back(itA->second);
				if (vecA == ittA->first) fA++;
			}
			ittA->second[i] = fA;
		}
		ittA++;
	}

	//Transpose and sort (canonical form of sign matrix) 
	vector<vector<int> > vssA;
	ittA = frequencyA.begin();
	while (ittA != frequencyA.end())
	{
		vector<int> vsA = ittA->second;
		vssA.push_back(vsA);
		ittA++;
	}
	vector<vector<int> > tvssA;
	vector<int> rowA;
	for (i = 0; i < vssA.size(); i++) rowA.push_back(0);
	for (j = 0; j < vssA[0].size(); j++) tvssA.push_back(rowA);
	for (i = 0; i < vssA.size(); i++)
		for (j = 0; j < vssA[0].size(); j++)
			tvssA[j][i] = vssA[i][j];
	for (i = 0; i < tvssA.size(); i++) tvssA[i].push_back(i);
	sort(tvssA.begin(), tvssA.end());


	//Determine equivalence classes k 
	vector<int> classA;
	vector<int> clA, dlA;
	int cA = 0;
	int icA = 0;
	dlA = tvssA[icA]; dlA.pop_back();
	while (icA < nA)
	{
		clA = tvssA[icA]; clA.pop_back();
		if (clA == dlA) classA.push_back(cA);
		else { dlA = clA; cA++; classA.push_back(cA); }
		icA++;
	}

	//Final Vertices 
	vector<int> vertexA;
	for (i = 0; i < nA; i++)
		vertexA.push_back(tvssA[i][tvssA[i].size() - 1]);

	//Final Sign Matrix 
	map<vector<int>, vector<int> > signmatrixA;
	for (i = 0; i < nA; i++)
		for (j = 0; j < nA; j++)
		{
			vector<int> indA;
			indA.push_back(vertexA[i]);
			indA.push_back(vertexA[j]);
			vector<int> siA;
			itA = dA.find(indA); siA.push_back(itA->second);
			itA = ntA.find(indA); siA.push_back(itA->second);
			itA = eA.find(indA); siA.push_back(itA->second);
			signmatrixA[indA] = siA;
		}


	//Process Graph B 
	int nB = rgraphB.size();

	//Initial sorting 
	vector<vector<int> > distanceB;
	vector<vector<int> > neighborB;
	for (i = 0; i < rgraphB.size(); i++)
	{
		vector<int> rowdistanceB, rowneighborB;
		for (j = 0; j < rgraphB[i].size(); j++)
		{
			if (rgraphB[i][j] == 1) rowneighborB.push_back(j);
			vector<int> indexdB;
			for (k = 0; k < rgraphB.size(); k++) indexdB.push_back(k);
			indexdB[0] = i; indexdB[i] = 0;
			vector<vector<int> > dgraphB = reindex(rgraphB, indexdB);
			vector<vector<int> > dpathB = dijkstra(dgraphB);
			int ddB = -1;
			for (k = 0; k < dpathB.size(); k++)
				if (inv(indexdB)[dpathB[k][dpathB[k].size() - 1]] == j)
				{
					ddB = dpathB[k].size() - 1; break;
				}
			rowdistanceB.push_back(ddB);
		}
		distanceB.push_back(rowdistanceB);
		neighborB.push_back(rowneighborB);
	}

	vector<vector<int> > sequenceB;
	for (i = 0; i < rgraphB.size(); i++)
	{
		vector<int> mutualdistanceB;
		for (j = 0; j < neighborB[i].size() - 1; j++)
			for (k = j + 1; k < neighborB[i].size(); k++)
				mutualdistanceB.push_back(distanceB[neighborB[i][j]][neighborB[i][k]]);
		sort(mutualdistanceB.begin(), mutualdistanceB.end());
		vector<int> tmutualB;
		for (k = 0; k < nB*nB - 1 - mutualdistanceB.size(); k++) tmutualB.push_back(0);
		for (k = 0; k < mutualdistanceB.size(); k++) tmutualB.push_back(mutualdistanceB[k]);
		sequenceB.push_back(tmutualB);
	}

	map<vector<int>, int> sorterB;
	for (i = 0; i < sequenceB.size(); i++) sorterB[sequenceB[i]] = i;
	vector<int> mainindexB;
	map<vector<int>, int>::iterator sitB = sorterB.begin();
	while (sitB != sorterB.end())
	{
		vector<int>vsitB = sitB->first;
		for (i = 0; i < sequenceB.size(); i++)
			if (sequenceB[i] == vsitB) mainindexB.push_back(i);
		sitB++;
	}

	vector< vector<int> > graphB = reindex(rgraphB, mainindexB);

	//Compute degree sequence 
	vector<int> degB;
	int sumB;
	for (i = 0; i < nB; i++)
	{
		sumB = 0;
		for (j = 0; j < nB; j++)
		{
			if (graphB[i][j] == 1) sumB++;
		}
		degB.push_back(sumB);
	}
	vector<int> sorted_degB = degB;
	sort(sorted_degB.begin(), sorted_degB.end());

	//Pair graphs 
	map<vector<int>, set<int> > PB;
	map<vector<int>, int> dB;
	map<vector<int>, int> ntB;
	map<vector<int>, int> eB;

	for (N1 = 0; N1 < graphB.size(); N1++)
		for (N2 = 0; N2 < graphB[N1].size(); N2++)
		{
			int signB = -1;
			vector<vector<int> > tgraphB = graphB;
			if (graphB[N1][N2] != 0) { tgraphB[N1][N2] = 0; tgraphB[N2][N1] = 0; signB = 1; }
			//Compute shortest paths from vertex 1  
			vector<vector<int> > shortest_path1B;
			vector<int> index1B; for (q = 0; q < nB; q++) index1B.push_back(q);
			index1B[0] = N1; index1B[N1] = 0;
			vector< vector<int> > temp_graph1B = reindex(tgraphB, index1B);
			vector< vector<int> > p1B = dijkstra(temp_graph1B);
			for (i = 0; i < p1B.size(); i++)
			{
				vector<int> tpath1B;
				for (j = 0; j < p1B[i].size(); j++) { tpath1B.push_back(inv(index1B)[p1B[i][j]]); }
				shortest_path1B.push_back(tpath1B);
			}
			//Compute shortest paths from vertex 2  
			vector<vector<int> > shortest_path2B;
			vector<int> index2B; for (q = 0; q < nB; q++) index2B.push_back(q);
			index2B[0] = N2; index2B[N2] = 0;
			vector< vector<int> > temp_graph2B = reindex(tgraphB, index2B);
			vector< vector<int> > p2B = dijkstra(temp_graph2B);
			for (i = 0; i < p2B.size(); i++)
			{
				vector<int> tpath2B;
				for (j = 0; j < p2B[i].size(); j++) { tpath2B.push_back(inv(index2B)[p2B[i][j]]); }
				shortest_path2B.push_back(tpath2B);
			}
			//Compute distance between vertex 1 and vertex 2 
			int DB = 0;
			for (i = 0; i < shortest_path1B.size(); i++)
				if (shortest_path1B[i][shortest_path1B[i].size() - 1] == N2)
				{
					DB = shortest_path1B[i].size() - 1; break;
				}
			//Compute shortest paths between vertex 1 and vertex 2 
			vector<vector<int> > shortest_path12B;
			for (i = 0; i < shortest_path1B.size(); i++)
				for (j = 0; j < shortest_path2B.size(); j++)
				{
					if (shortest_path1B[i][shortest_path1B[i].size() - 1] ==
						shortest_path2B[j][shortest_path2B[j].size() - 1])
					{
						vector<int> temppathB = shortest_path1B[i];
						for (k = shortest_path2B[j].size() - 2; k >= 0; k--)
							temppathB.push_back(shortest_path2B[j][k]);
						if (temppathB.size() - 1 == DB)
							shortest_path12B.push_back(temppathB);
					}
				}
			//Pair graph for vertex 1 and vertex 2 
			bool checkB = false;
			set<int> SB;
			for (i = 0; i < shortest_path12B.size(); i++)
			{
				if (shortest_path12B[i][0] == N1 &&
					shortest_path12B[i][shortest_path12B[i].size() - 1] == N2)
				{
					checkB = true;
					for (j = 0; j < shortest_path12B[i].size(); j++)
						SB.insert(shortest_path12B[i][j]);
				}
			}
			vector<int> VB; VB.push_back(N1); VB.push_back(N2);
			PB[VB] = SB;
			//Distance between vertex 1 and vertex 2 
			if (checkB) dB[VB] = signB * DB;
			else dB[VB] = signB;
			//Count number of vertices in pair graph 
			ntB[VB] = SB.size();
			//Count number of edges in pair graph 
			int countB = 0;
			for (i = 0; i < nB; i++)
				for (j = i + 1; j < nB; j++)
				{
					bool findpairB = false;
					set<int>::iterator i1B, i2B;
					i1B = SB.find(i); i2B = SB.find(j);
					if (i1B != SB.end() && i2B != SB.end()) findpairB = true;
					if (findpairB && graphB[i][j] != 0) countB++;
				}
			eB[VB] = countB;
		}

	//Make frequency table (sign frequency vectors) 
	map<vector<int>, int>::iterator itB;
	map<vector<int>, vector<int> > frequencyB;
	vector<int> dummyB; for (i = 0; i < nB; i++) dummyB.push_back(0);
	for (i = 0; i < nB; i++)
	{
		for (j = 0; j < nB; j++)
		{
			vector<int> vecB;
			vector<int> indB; indB.push_back(i); indB.push_back(j);
			itB = dB.find(indB);
			vecB.push_back(itB->second);
			itB = ntB.find(indB);
			vecB.push_back(itB->second);
			itB = eB.find(indB);
			vecB.push_back(itB->second);
			frequencyB[vecB] = dummyB;
		}
	}

	map<vector<int>, vector<int> >::iterator ittB = frequencyB.begin();
	while (ittB != frequencyB.end())
	{
		for (i = 0; i < nB; i++)
		{
			int fB = 0;
			for (j = 0; j < nB; j++)
			{
				vector<int> vecB;
				vector<int> indB; indB.push_back(i); indB.push_back(j);
				itB = dB.find(indB);
				vecB.push_back(itB->second);
				itB = ntB.find(indB);
				vecB.push_back(itB->second);
				itB = eB.find(indB);
				vecB.push_back(itB->second);
				if (vecB == ittB->first) fB++;
			}
			ittB->second[i] = fB;
		}
		ittB++;
	}

	//Transpose and sort (canonical form of sign matrix) 
	vector<vector<int> > vssB;
	ittB = frequencyB.begin();
	while (ittB != frequencyB.end())
	{
		vector<int> vsB = ittB->second;
		vssB.push_back(vsB);
		ittB++;
	}
	vector<vector<int> > tvssB;
	vector<int> rowB;
	for (i = 0; i < vssB.size(); i++) rowB.push_back(0);
	for (j = 0; j < vssB[0].size(); j++) tvssB.push_back(rowB);
	for (i = 0; i < vssB.size(); i++)
		for (j = 0; j < vssB[0].size(); j++)
			tvssB[j][i] = vssB[i][j];
	for (i = 0; i < tvssB.size(); i++) tvssB[i].push_back(i);
	sort(tvssB.begin(), tvssB.end());

	//Determine equivalence classes k 
	vector<int> classB;
	vector<int> clB, dlB;
	int cB = 0;
	int icB = 0;
	dlB = tvssB[icB]; dlB.pop_back();
	while (icB < nB)
	{
		clB = tvssB[icB]; clB.pop_back();
		if (clB == dlB) classB.push_back(cB);
		else { dlB = clB; cB++; classB.push_back(cB); }
		icB++;
	}

	//Final Vertices 


	vector<int> vertexB;
	for (i = 0; i < nB; i++)
		vertexB.push_back(tvssB[i][tvssB[i].size() - 1]);

	//Final Sign Matrix 
	map<vector<int>, vector<int> > signmatrixB;
	for (i = 0; i < nB; i++)
		for (j = 0; j < nB; j++)
		{
			vector<int> indB;
			indB.push_back(vertexB[i]);
			indB.push_back(vertexB[j]);
			vector<int> siB;
			itB = dB.find(indB); siB.push_back(itB->second);
			itB = ntB.find(indB); siB.push_back(itB->second);
			itB = eB.find(indB); siB.push_back(itB->second);
			signmatrixB[indB] = siB;
		}

	//Isomorphism Index 
	vector<int> fixisoB, isoB;
	for (i = 0; i < nB; i++) fixisoB.push_back(i); isoB = fixisoB;
	bool isomorphic = false;
	bool possibly_isomorphic = false;
	if (sorted_degA == sorted_degB) {
		vector<vector<int> > checksignA;
		ittA = frequencyA.begin();
		while (ittA != frequencyA.end())
		{
			vector<int> checksignrowA;
			vector<int> wsA = ittA->first;
			for (i = 0; i < wsA.size(); i++) checksignrowA.push_back(wsA[i]);
			vector<int> vsA = ittA->second;
			for (i = 0; i < vsA.size(); i++) checksignrowA.push_back(vsA[vertexA[i]]);
			checksignA.push_back(checksignrowA);
			ittA++;
		}
		vector<vector<int> > checksignB;
		ittB = frequencyB.begin();
		while (ittB != frequencyB.end())
		{
			vector<int> checksignrowB;
			vector<int> wsB = ittB->first;
			for (i = 0; i < wsB.size(); i++) checksignrowB.push_back(wsB[i]);
			vector<int> vsB = ittB->second;
			for (i = 0; i < vsB.size(); i++) checksignrowB.push_back(vsB[vertexB[i]]);
			checksignB.push_back(checksignrowB);
			ittB++;
		}
		if (checksignA == checksignB) { possibly_isomorphic = true; }
	}

	if (possibly_isomorphic)
	{
		//Find isomorphism 
		for (int J = 0; J < nB; J++)
		{
			if (isomorphic) break;
			isoB = fixisoB; isoB[0] = fixisoB[J]; isoB[J] = fixisoB[0];
			for (int I = 0; I < nB*nB; I++)
			{
				vector<int> oldisoB = isoB;
				isoB = transform(signmatrixA, signmatrixB, vertexA, vertexB, isoB);


				bool quit = false, mismatch = false;
				for (int ii = 0; ii < nB; ii++)
				{
					if (quit) break;
					for (int jj = ii + 1; jj < nB; jj++)
					{
						vector<int> tindA, ta;
						tindA.push_back(vertexA[ii]); tindA.push_back(vertexA[jj]);
						vector<int> tindB, tb;
						tindB.push_back(vertexB[isoB[ii]]);
						tindB.push_back(vertexB[isoB[jj]]);
						ittA = signmatrixA.find(tindA); ta = ittA->second;


						ittB = signmatrixB.find(tindB); tb = ittB->second;
						if (ta != tb) { mismatch = true; quit = true; break; }
					}
				}
				if (isoB == oldisoB)
				{
					if (!mismatch) isomorphic = true;
					break;
				}
			}
		}

	}

	if (!possibly_isomorphic)
	{
		return false;
	}
	if (possibly_isomorphic && !isomorphic)
	{
		return false;
	}
	if (possibly_isomorphic && isomorphic)
	{
		return true;
	}

	return true;
}



//Functions 

vector<vector<int> > dijkstra(vector<vector<int> > graph)
{
	vector<vector<int> > table;
	int i, j, k, n = graph.size();
	//Initialize Table 
	const int infinity = n;
	vector<bool> known;
	for (i = 0; i < n; i++) known.push_back(false);
	vector<int> d;
	d.push_back(0);
	for (i = 1; i < n; i++) d.push_back(infinity);
	vector<int> p;
	for (i = 0; i < n; i++) p.push_back(-1);
	//End initialization 

	//Iteration 
	for (k = 0; k < n; k++)
	{
		//Find min of d for unknown vertices 
		int min = 0;
		while (known[min] == true)min++;
		for (i = 0; i < n; i++)
			if (known[i] == false && d[i] < d[min])min = i;
		//End find 
		//Update Table 
		known[min] = true;
		for (j = 0; j < n; j++)
		{
			if (graph[min][j] != 0 && d[j] > d[min] + graph[min][j] && known[j] == false)
			{
				d[j] = d[min] + graph[min][j];
				p[j] = min;
			}
		}
		//End update 
	}
	table.push_back(d);
	table.push_back(p);
	vector<vector<int> > path;
	for (i = 0; i < n; i++)
	{
		vector<int> temp_path;
		vector<int> temp;
		k = i;
		while (k != -1)
		{
			temp.push_back(k);
			k = table[1][k];
		}
		temp_path.push_back(temp[temp.size() - 1]);
		for (j = temp.size() - 2; j >= 0; j--)
		{
			temp_path.push_back(temp[j]);
		}
		path.push_back(temp_path);
	}
	return path;
}

vector<vector<int> > reindex(vector<vector<int> > graph, vector<int> index)
{
	vector<vector<int> > temp_graph = graph;
	for (int i = 0; i < graph.size(); i++)
		for (int j = 0; j < graph[i].size(); j++)
			temp_graph[index[i]][index[j]] = graph[i][j];
	return temp_graph;
}

vector<int> inv(vector<int> index)
{
	vector<int> inverse = index;
	for (int i = 0; i < index.size(); i++)
		inverse[index[i]] = i;
	return inverse;
}

vector<int> transform(map<vector<int>, vector<int> > signmatrixA,
	map<vector<int>, vector<int> > signmatrixB,
	vector<int> vertexA, vector<int> vertexB,
	vector<int> isoB)
{
	vector<int> iso = isoB;
	map<vector<int>, vector<int> >::iterator it;
	int k, n = iso.size();
	bool found = false;
	bool check = true;
	for (int i = 0; i < n; i++)
	{
		if (found) break;
		for (int j = i + 1; j < n; j++)
		{
			vector<int> indA, a;
			indA.push_back(vertexA[i]); indA.push_back(vertexA[j]);
			vector<int> indB, b;

			indB.push_back(vertexB[isoB[i]]);
			indB.push_back(vertexB[isoB[j]]);
			it = signmatrixA.find(indA); a = it->second;
			it = signmatrixB.find(indB); b = it->second;
			if (a != b)
			{
				k = j;
				vector<int> temp_ind = indB;
				while (k < n - 1 && check == true)
				{
					k++;
					temp_ind[1] = vertexB[isoB[k]];
					it = signmatrixB.find(temp_ind); b = it->second;
					//check 
					if (a == b)
					{
						vector<int> temp_iso = isoB;
						temp_iso[j] = isoB[k];  temp_iso[k] = isoB[j];
						int ti = -1, tj = -1;
						bool quit = false;
						for (int ii = 0; ii < n; ii++)
						{
							if (quit) break;
							for (int jj = ii + 1; jj < n; jj++)
							{
								vector<int> tindA, ta;
								tindA.push_back(vertexA[ii]); tindA.push_back(vertexA[jj]);
								vector<int> tindB, tb;
								tindB.push_back(vertexB[temp_iso[ii]]);
								tindB.push_back(vertexB[temp_iso[jj]]);
								it = signmatrixA.find(tindA); ta = it->second;
								it = signmatrixB.find(tindB); tb = it->second;
								if (ta != tb) { ti = ii; tj = jj; quit = true; break; }
								if (k == n - 1 && ti == -1) { check = false; quit = true; break; }
							}
						}
						if (ti == -1 || ti > i || (ti == i && tj > j)) check = false;
					}
					//end check  
				}
				if (!check) {
					found = true; iso[j] = isoB[k]; iso[k] = isoB[j];
					break;
				}
				if (check) return iso;
			}
		}
	}
	return iso;
}