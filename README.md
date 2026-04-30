# 🚀 Epoll-Based Load Balancer in C

## 📌 Overview
This project implements a high-performance TCP load balancer in C using the Linux `epoll` API. It distributes incoming client connections across multiple backend servers using a **least-connections (greedy) algorithm**.

---

## ⚙️ Features
- Load balancing using least active connections
- Efficient I/O handling with `epoll`
- Non-blocking sockets
- Bidirectional data forwarding (client ↔ server)
- Tracks active connections per backend

---

## 🧠 How It Works
1. Load balancer listens on **port 9000**
2. Accepts incoming client connections
3. Selects backend server with least active connections
4. Connects client to selected backend
5. Uses `epoll` to forward data between client and backend
6. Updates connection count when connection closes

---

## 🏗️ Architecture

Client → Load Balancer (Port 9000) → Backend Servers  
├── 127.0.0.1:8001  
├── 127.0.0.1:8002  
└── 127.0.0.1:8003  

---

## 📁 File Structure
load_balancer.c  
README.md  

---

## 🔧 Requirements
- Linux OS (epoll is Linux-specific)
- GCC compiler

---

## ⚡ Compilation
gcc load_balancer.c -o load_balancer

---

## ▶️ Run
./load_balancer

---

## 🖥️ Backend Setup
Run backend servers on:

- 127.0.0.1:8001  
- 127.0.0.1:8002  
- 127.0.0.1:8003  

Example using netcat:
nc -l 8001  
nc -l 8002  
nc -l 8003  

---

## 🧪 Testing
Connect clients:
nc localhost 9000

You should see load distribution:
Client → Server 0  
Client → Server 1  
Client → Server 2  

---

## ⚠️ Limitations
- Blocking `connect()` used
- No backend health checks
- No HTTP-level parsing (TCP only)
- Basic memory management

---

## 🚀 Future Improvements
- Non-blocking backend connections
- Health checks for servers
- Round-robin / weighted balancing
- Logging and monitoring
- HTTP-aware routing

---

## 🧑‍💻 Author
This project demonstrates:
- Socket programming in C
- Event-driven architecture
- Load balancing techniques
