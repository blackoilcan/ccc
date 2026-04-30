# High-Performance Load Balancer in C (Epoll)

## Description
This project is a TCP Load Balancer written in C using the Linux `epoll` API.  
It distributes incoming client connections across multiple backend servers using a greedy (least-connections) algorithm.

---

## Key Highlights
- epoll-based scalable I/O
- Non-blocking sockets
- Full-duplex communication
- Efficient handling of multiple concurrent connections

---

## Core Concept: Greedy Load Balancing Algorithm

The central idea of this project is the use of a **Greedy Algorithm** for load balancing.

### What is the Greedy Approach?
A greedy algorithm always makes the best possible decision at the current step without considering future consequences.

In this project:
- The "best" choice = server with the **least number of active connections**
- This decision is made **for every incoming client request**

---

### Algorithm Logic

For each new connection:
1. Check active connections of all backend servers
2. Select the server with the minimum connections
3. Assign the client to that server
4. Increment its active connection count

---

### Example

| Server | Active Connections |
|--------|-------------------|
| S1     | 5                 |
| S2     | 2                 |
| S3     | 4                 |

New request is assigned to **S2** because it has the least load.

---

### Implementation in Code

```c
int select_server() {
    int min = 0;
    for (int i = 1; i < MAX_SERVERS; i++) {
        if (servers[i].active_connections < servers[min].active_connections)
            min = i;
    }
    return min;
}
