#pragma region HEADERS
#include <iostream>
#include <stack>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <chrono>
#include <random>

using namespace std;
#pragma endregion

#pragma region DECLARATIONS
struct Strokes {
	int n;
	int h;
	int v;

	Strokes(int n, int h, int v)
	{
		this->n = n;
		this->h = h;
		this->v = v;
	}
};

string output;
vector<int> testSizes = { 1000, 10000, 100000, 1000000, 10000000, 100000000, 400000000 };

void test(int size, string type);
int paintingFenceOrig(vector<int>& a, int left, int right, int curHeight);
int paintingFenceSegmentTree(const vector<int>& a);
int paintingFenceCartesianTree(const std::vector<int>& a, int N);
int paintingFence(vector<int>& a);
#pragma endregion

#pragma region MAIN
int main()
{
	for (const auto& size : testSizes)
	{
		output.clear();
		output += "SIZE = " + to_string(size) + "\n\n";
		
		test(size, "increasing");
		test(size, "decreasing");
		test(size, "randomized");
		
		output += "=================\n\n";
		cout << output;
	}
}
#pragma endregion

#pragma region TESTING
void test(int size, string type)
{
	vector<int> a(size);
	if (type == "increasing")
	{
		for (int i = 0; i < size; i++)
			a[i] = i + 1;
	}
	else if (type == "decreasing")
	{
		for (int i = 0; i < size; i++)
			a[i] = size - i;
	}
	else
	{
		const double PI = acos(-1.0);
		int noiseRange = 20;
		int heightBase = 20;
		int peakInterval = 100;

		mt19937 gen(69);
		uniform_int_distribution<> noiseDist(-noiseRange, noiseRange);
		uniform_int_distribution<> peakDist(heightBase * 2, heightBase * 4);
		uniform_int_distribution<> intervalDist(1, peakInterval * 2);

		int nextPeak = intervalDist(gen);
		for (int i = 0; i < size; i++) 
		{
			double wave = sin(2.0 * PI * i / 50.0) * (heightBase / 2);
			int h = heightBase + int(wave);
			h += noiseDist(gen);

			if (--nextPeak <= 0) 
			{
				h += peakDist(gen);
				nextPeak = intervalDist(gen);
			}

			a[i] = max(1, h);
		}
	}

	auto start = chrono::high_resolution_clock::now();

	int minStrokesOrig = 0;
	if (size < 100000)
		minStrokesOrig = paintingFenceOrig(a, 0, a.size() - 1, 0);
	else if (type == "randomized")
		minStrokesOrig = paintingFenceOrig(a, 0, a.size() - 1, 0);


	auto orig = chrono::high_resolution_clock::now();

	int minStrokesTree = 0;
	if (size < 100000)
		minStrokesTree = paintingFenceSegmentTree(a);
	else if (size < 1000000 && type == "randomized")
		minStrokesTree = paintingFenceSegmentTree(a);
	auto tree = chrono::high_resolution_clock::now();

	int minStrokesCart = 0;
	if (size < 100000)
		minStrokesCart = paintingFenceCartesianTree(a, a.size());
	else if (size < 1000000 && type == "randomized")
		minStrokesCart = paintingFenceCartesianTree(a, a.size());

	auto cart = chrono::high_resolution_clock::now();

	int minStrokesFast = paintingFence(a);
	auto fast = chrono::high_resolution_clock::now();

	long long origTime = chrono::duration_cast<chrono::microseconds>(orig - start).count();
	long long treeTime = chrono::duration_cast<chrono::microseconds>(tree - orig).count();
	long long cartTime = chrono::duration_cast<chrono::microseconds>(cart - tree).count();
	long long fastTime = chrono::duration_cast<chrono::microseconds>(fast - cart).count();

	output += "=================\nTEST (" + type + ")\n=================\n";
	
	if (minStrokesOrig)
		output += "Min Strokes ORIG = " + to_string(minStrokesOrig) + "\tTime: " + to_string(origTime) + " microseconds\n";
	else
		output += "Min Strokes ORIG = STACK OVERFLOW\n";

	if (minStrokesTree)
		output += "Min Strokes SEGT = " + to_string(minStrokesTree) + "\tTime: " + to_string(treeTime) + " microseconds\n";
	else
		output += "Min Strokes SEGT = STACK OVERFLOW\n";

	if (minStrokesCart)
		output += "Min Strokes CART = " + to_string(minStrokesCart) + "\tTime: " + to_string(cartTime) + " microseconds\n";
	else
		output += "Min Strokes CART = STACK OVERFLOW\n";
	
	output += "Min Strokes O(N) = " + to_string(minStrokesFast) + "\tTime: " + to_string(fastTime) + " microseconds\n";
	
	a.clear();
	a.reserve(0);
}
#pragma endregion

#pragma region D/C (NAIVE)
int paintingFenceOrig(vector<int>& a, int left, int right, int curHeight)
{
	int minHeight = INT_MAX;

	for (int i = left; i <= right; i++)
	{
		if (a[i] < minHeight)
			minHeight = a[i];
	}

	int strokes = minHeight - curHeight;

	int nextLeft = -1;
	for (int i = left; i <= right; i++)
	{
		if (a[i] > minHeight && nextLeft == -1)
			nextLeft = i;
		else if (a[i] == minHeight && nextLeft != -1)
		{
			strokes += paintingFenceOrig(a, nextLeft, i - 1, minHeight);
			nextLeft = -1;
		}
	}

	if (nextLeft != -1)
		strokes += paintingFenceOrig(a, nextLeft, right, minHeight);

	return min(strokes, right - left + 1);
}
#pragma endregion

#pragma region D/C (SEGMENT TREE)
vector<int> segTree;
const vector<int>* arrPtr;

void buildSeg(int p, int L, int R) 
{
	if (L == R)
		segTree[p] = L;
	else 
	{
		int M = (L + R) >> 1;
		buildSeg(p << 1, L, M);
		buildSeg(p << 1 | 1, M + 1, R);
		int i = segTree[p << 1], j = segTree[p << 1 | 1];
		segTree[p] = ((*arrPtr)[i] < (*arrPtr)[j] ? i : j);
	}
}

int querySeg(int p, int L, int R, int ql, int qr) 
{
	if (qr < L || ql > R) return -1;
	if (ql <= L && R <= qr) return segTree[p];
	int M = (L + R) >> 1;
	int left = querySeg(p << 1, L, M, ql, qr);
	int right = querySeg(p << 1 | 1, M + 1, R, ql, qr);
	if (left < 0) return right;
	if (right < 0) return left;
	return ((*arrPtr)[left] < (*arrPtr)[right] ? left : right);
}

int solveRec(int l, int r, int h) 
{
	if (l > r) return 0;
	int m = querySeg(1, 0, int(arrPtr->size()) - 1, l, r);
	int vertical = r - l + 1;
	int horizontal = ((*arrPtr)[m] - h)
		+ solveRec(l, m - 1, (*arrPtr)[m])
		+ solveRec(m + 1, r, (*arrPtr)[m]);
	return std::min(vertical, horizontal);
}

int paintingFenceSegmentTree(const std::vector<int>& a) 
{
	int n = int(a.size());
	arrPtr = &a;
	segTree.assign(4 * n, 0);
	buildSeg(1, 0, n - 1);
	return solveRec(0, n - 1, 0);
}
#pragma endregion

#pragma region CARTESIAN TREE
struct TreeNode
{
	int idx;
	int value;
	int left;
	int right;

	TreeNode(int value, int idx) : value(value), idx(idx), left(-1), right(-1) {}
};

vector<TreeNode> treeNodes;

TreeNode BuildCartesianTree(const vector<int> &arr, int N)
{
	TreeNode *root = nullptr;
	treeNodes.clear();
	treeNodes.reserve(N);

	for (int i = 0; i < N; i++)
		treeNodes.emplace_back(arr[i], i);

	vector<TreeNode> stack;
	stack.reserve(N);

	for (int i = 0; i < N; i++) 
	{
		TreeNode &currentNode = treeNodes[i];

		while (!stack.empty() && stack.back().value > currentNode.value)
			currentNode.left = stack.back().idx, stack.pop_back();

		if (!stack.empty())
			treeNodes[stack.back().idx].right = currentNode.idx;
		else
			root = &currentNode;

		stack.push_back(currentNode);
	}

	return *root;
}

int MinStrokesCartesian(int l, int r, int height, int currentNodeIdx)
{
	if (l > r)
		return 0;

	TreeNode currentNode = treeNodes[currentNodeIdx];

	if (r > l)
	{
		int takeCur = currentNode.value - height +
			MinStrokesCartesian(l, currentNode.idx - 1, currentNode.value, currentNode.left) +
			MinStrokesCartesian(currentNode.idx + 1, r, currentNode.value, currentNode.right);

		return min(takeCur, r - l + 1);
	}

	return height >= currentNode.value ? 0 : 1;
}

int paintingFenceCartesianTree(const std::vector<int>& a, int N)
{
	TreeNode root = BuildCartesianTree(a, N);
	return MinStrokesCartesian(0, N - 1, 0, root.idx);
}
#pragma endregion

#pragma region O(N)
int paintingFence(vector<int>& a)
{
	vector<Strokes> stack;
	stack.reserve(a.size());

	Strokes s(0, a[0], 1);

	for (int i = 1, sz = a.size(); i < sz; i++)
	{
		if (a[i] == s.h)
			s.v++;
		else if (a[i] > s.h)
		{
			stack.push_back(s);
			s.n = 0;
			s.h = a[i];
			s.v = 1;
		}
		else
		{
			if (stack.empty())
			{
				s.n = min(s.v, s.n + s.h - a[i]);
				s.h = a[i];
				s.v++;
			}
			else
			{
				Strokes top = stack.back();
				
				if (top.h < a[i])
				{
					s.n = min(s.v, s.n + s.h - a[i]);
					s.h = a[i];
					s.v++;
				}
				else
				{
					s.n = top.n + min(s.v, s.n + s.h - top.h);
					s.h = top.h;
					s.v += top.v;
					stack.pop_back();
					i--;
				}
			}
		}
	}

	int stackSz = stack.size();
	while (stackSz--)
	{
		Strokes top = stack.back();
		stack.pop_back();

		s.n = top.n + min(s.v, s.n + s.h - top.h);
		s.h = top.h;
		s.v += top.v;
	}

	return min(s.v, s.n + s.h);
}
#pragma endregion
