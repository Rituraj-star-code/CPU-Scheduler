# CPU Scheduler Simulator

A web-based **CPU Scheduling Simulator** that demonstrates and visualizes different CPU scheduling algorithms.
The project helps users understand how processes are scheduled by the CPU and how different scheduling algorithms affect **waiting time, turnaround time, response time, completion time, and starvation**.

## 🚀 Features

* Add multiple processes with:

  * Process ID
  * Arrival Time
  * Burst Time
  * Priority
* Simulate CPU scheduling algorithms
* Generate a **Gantt Chart** for visualization
* Calculate important scheduling metrics:

  * Completion Time (CT)
  * Turnaround Time (TAT)
  * Waiting Time (WT)
  * Response Time (RT)
* Detect possible **starvation**
* Display scheduling results in a structured table
* Compare the performance of different scheduling algorithms
* Interactive and easy-to-use web interface

## 🧠 Scheduling Algorithms

The simulator can be used to understand different CPU scheduling techniques such as:

### 1. First Come First Serve (FCFS)

Processes are executed in the order in which they arrive.

**Type:** Non-preemptive

### 2. Shortest Job First (SJF)

The process with the shortest burst time is selected first.

**Type:** Non-preemptive

### 3. Shortest Remaining Time First (SRTF)

A preemptive version of SJF where the process with the shortest remaining burst time gets the CPU.

**Type:** Preemptive

### 4. Round Robin (RR)

Each process gets a fixed amount of CPU time called a **time quantum**.

**Type:** Preemptive

### 5. Priority Scheduling

Processes are executed according to their priority.

It can be implemented as either preemptive or non-preemptive scheduling.

### 6. Multilevel Feedback Queue (MLFQ)

Processes move between different priority queues depending on their CPU behavior.

MLFQ helps demonstrate how modern operating systems can balance **response time, throughput, and starvation prevention**.

## 📊 Scheduling Metrics

For every process, the simulator calculates:

### Completion Time (CT)

The time at which a process finishes execution.

### Turnaround Time (TAT)

```text
TAT = CT - Arrival Time
```

### Waiting Time (WT)

```text
WT = TAT - Burst Time
```

### Response Time (RT)

```text
RT = First CPU Start Time - Arrival Time
```

These metrics can be used to compare the performance of different scheduling algorithms.

## ⚠️ Starvation Detection

The simulator also provides a mechanism to identify processes that may suffer from **starvation**.

A process can be marked as starved when its waiting time crosses a predefined starvation threshold.

This is particularly useful for understanding starvation in algorithms such as:

* Priority Scheduling
* SJF
* Multilevel Feedback Queue

## 📈 Gantt Chart

The simulator provides a visual representation of CPU execution using a Gantt Chart.

Example:

```text
| P1 | P2 | P3 | P1 |
0    4    7    9    12
```

This makes it easier to understand:

* CPU utilization
* Process execution order
* Waiting time
* Completion time
* Preemption

## 🛠️ Technologies Used

* **HTML** – Structure of the web application
* **CSS** – Styling and user interface
* **JavaScript** – Scheduling logic and dynamic result generation
* **Git & GitHub** – Version control and project hosting

## 📂 Project Structure

```text
CPU-Scheduler/
│
├── index.html
├── style.css
├── script.js
│
├── assets/
│   └── ...
│
└── README.md
```

> The exact file structure may vary depending on the current version of the project.

## ▶️ How to Run

### Option 1: Run directly in browser

1. Clone the repository:

```bash
git clone https://github.com/Rituraj-star-code/CPU-Scheduler.git
```

2. Open the project folder.

3. Open `index.html` in your browser.

### Option 2: Using VS Code

1. Open the project in **VS Code**.
2. Install the **Live Server** extension.
3. Right-click `index.html`.
4. Select **Open with Live Server**.

The project will open in your browser.

## 🔄 How It Works

The basic workflow of the simulator is:

```text
Enter Process Details
        ↓
Select Scheduling Algorithm
        ↓
Apply Scheduling Logic
        ↓
Generate Execution Sequence
        ↓
Calculate CT, TAT, WT and RT
        ↓
Check for Starvation
        ↓
Display Gantt Chart + Results
```

## 🎯 Learning Objectives

This project was developed to understand the practical implementation of **CPU scheduling concepts in Operating Systems**.

It helps demonstrate:

* Process scheduling
* Preemptive vs Non-preemptive scheduling
* Scheduling algorithms
* Gantt charts
* Process synchronization concepts
* Scheduling performance metrics
* Starvation
* Trade-offs between different scheduling algorithms

## 🔮 Future Improvements

Some possible future improvements include:

* Add more scheduling algorithms
* Add algorithm performance comparison charts
* Add CPU utilization calculation
* Add throughput calculation
* Improve starvation prevention
* Add context-switching overhead
* Add detailed algorithm animations
* Add dark/light mode
* Improve mobile responsiveness

## 👨‍💻 Author

**Rituraj Mishra**

ECE Student | C++ & DSA Enthusiast

---

⭐ If you find this project useful, consider giving the repository a star!
