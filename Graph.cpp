#include "Graph.h"
#include <algorithm>
#include <atomic>
#include <barrier>
#include <boost/sort/spreadsort/integer_sort.hpp>
#include <cstring>
#include <queue>
#include <thread>
#include <cassert>

Graph::Graph(int vertices) : V(vertices), adjList(vertices), dists(vertices), pdists(vertices) {}

void Graph::addEdge(int src, int dest) {
    if (src < 0 || dest < 0 || src >= V || dest >= V) return;
    auto& vec = adjList[src];
    if (std::find(vec.begin(), vec.end(), dest) == vec.end()) {
        vec.push_back(dest);
    }
}

bool Graph::checkDistances() const {
    return std::memcmp(dists.data(), pdists.data(), V * sizeof(int)) == 0;
}

std::vector<int>& Graph::neighbours(int v) {
    return adjList[v];
}

static constexpr int EmptyNode  = (2e7 + 1);
static constexpr int BatchSize  = 256;

struct Frontier {
    std::vector<int> vertices;
    std::atomic<int> head;

    Frontier(size_t count) : head(0) {
        vertices.reserve(count);
    }

    std::pair<int, int> getBatch() {
        int left = head.fetch_add(BatchSize);
        int right = left + BatchSize;
        if (right > (int)vertices.size()) {
            right = (int)vertices.size();
        }
        return {left, right};
    }

    void updateBatch(std::pair<int, int>& seg, int v) {
        if (seg.first >= seg.second) {
            seg = getBatch();
        }
        vertices[seg.first++] = v;
    }
};

typedef std::deque<std::atomic<char>> VisitedType;

void Graph::parallelBFS(int startVertex) {
    ssize_t thread_num = std::thread::hardware_concurrency();
    VisitedType visited(V);
    size_t max_size = V + thread_num * BatchSize;
    Frontier current_level(max_size), next_level(max_size);

    next_level.vertices.resize(max_size);
    visited[startVertex].store(1);
    current_level.vertices.push_back(startVertex);
    int level = 1;

    std::barrier b{thread_num, [&](){
        boost::sort::spreadsort::integer_sort(next_level.vertices.begin(), next_level.vertices.begin() + next_level.head);
        while (next_level.head > 0 && next_level.vertices[next_level.head - 1] == EmptyNode) {
            --next_level.head;
        }
        for (auto it = next_level.vertices.begin(); it != next_level.vertices.begin() + next_level.head; ++it) {
            pdists[*it] = level;
        }
        ++level;
        std::swap(current_level.vertices, next_level.vertices);
        current_level.vertices.resize(next_level.head);
        next_level.vertices.resize(max_size, EmptyNode);
        next_level.head = 0;
        current_level.head = 0;
    }};

    std::vector<std::thread> threads;
    for (ssize_t i = 0; i < thread_num; ++i) {
        threads.emplace_back([&](){
            while (!current_level.vertices.empty()) {
                std::pair<int, int> writeBatch = {0, 0};
                for (;;) {
                    std::pair<int, int> readBatch = current_level.getBatch();
                    if (readBatch.first >= readBatch.second) {
                        break;
                    }
                    auto first_it = current_level.vertices.begin();
                    for (auto it = first_it + readBatch.first; it != first_it + readBatch.second; ++it) {
                        assert(*it != EmptyNode);
                        for (int v: neighbours(*it)) {
                            char exp = 0;
                            if (visited[v].compare_exchange_strong(exp, 1, std::memory_order_acq_rel)) {
                                next_level.updateBatch(writeBatch, v);
                            }
                        }
                    }
                }
                for (int i = writeBatch.first; i < writeBatch.second; ++i) {
                    next_level.vertices[i] = EmptyNode;
                }
                b.arrive_and_wait();
            }
        });
    }
    for (int i = 0; i < threads.size(); ++i)
        threads[i].join();
    
}

void Graph::bfs(int startVertex) {
    if (startVertex < 0 || startVertex >= V) return;
    std::vector<char> visited(V, 0);
    std::queue<int> q;

    visited[startVertex] = 1;
    q.push(startVertex);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int n : adjList[u]) {
            if (!visited[n]) {
                visited[n] = 1;
                dists[n] = dists[u] + 1;
                q.push(n);
            }
        }
    }
}

int Graph::vertices() const { return V; }