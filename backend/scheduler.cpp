#include <bits/stdc++.h>
using namespace std;    

struct Process {
    int pid;
    int at;
    int bt;
    int priority;

    int remaining;
    int ct = 0;
    int wt = 0;
    int tat = 0;
    int rt = -1;

    bool starved = false;
};

struct GanttBlock {
    string type; // PROCESS, IDLE, CS
    int pid;
    int start;
    int end;
};

vector<Process> processes;
vector<GanttBlock> gantt;

int contextSwitches = 0;
int contextSwitchTime = 0;

void addBlock(string type, int pid, int start, int end) {

    if (start >= end)
        return;

    // Merge consecutive blocks of same process
    if (!gantt.empty() &&
        gantt.back().type == type &&
        gantt.back().pid == pid &&
        gantt.back().end == start) {

        gantt.back().end = end;
        return;
    }

    gantt.push_back({
        type,
        pid,
        start,
        end
    });
}

void addIdle(int start, int end) {
    addBlock("IDLE", -1, start, end);
}

void addContextSwitch(int start, int duration) {

    if (duration <= 0)
        return;

    addBlock("CS", -1, start, start + duration);
    contextSwitchTime += duration;
}

void checkStarvation(
    vector<Process>& processes,
    int currentTime,
    int threshold
) {
    for (auto &p : processes) {

        // Process aa chuka hai aur complete nahi hua
        if (p.at <= currentTime &&
            p.remaining > 0) {

            int waitingTime = currentTime - p.at - (p.bt - p.remaining);

            if (waitingTime >= threshold) {
                p.starved = true;
            }
        }
    }
}

int getMLQLevel(const Process& p) {

    if (p.priority <= 2)
        return 0;

    if (p.priority <= 5)
        return 1;

    return 2;
}

void runMLQ(vector<Process>& p, int csTime, int starvationThreshold) {

    int n = p.size();
    int time = 0;
    int completed = 0;

    vector<queue<int>> q(3);
    vector<bool> added(n, false);

    int previousPid = -1;

    while (completed < n) {

        // Add newly arrived processes
        for (int i = 0; i < n; i++) {

            if (!added[i] && p[i].at <= time) {

                int level = getMLQLevel(p[i]);

                q[level].push(i);
                added[i] = true;
            }
        }

        int idx = -1;
        int level = -1;

        // Highest queue first
        for (int i = 0; i < 3; i++) {

            if (!q[i].empty()) {

                idx = q[i].front();
                q[i].pop();

                level = i;

                break;
            }
        }

        // CPU idle
        if (idx == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {

                if (!added[i]) {
                    nextArrival =
                        min(nextArrival, p[i].at);
                }
            }

            if (nextArrival != INT_MAX &&
                time < nextArrival) {

                addIdle(time, nextArrival);
                time = nextArrival;
            }

            previousPid = -1;
            continue;
        }

        Process &current = p[idx];

        // Context switch
        if (previousPid != -1 &&
            previousPid != current.pid &&
            csTime > 0) {

            addContextSwitch(time, csTime);

            time += csTime;

            contextSwitches++;
        }

        // Response time
        if (current.rt == -1) {

            current.rt =
                time - current.at;
        }

        int quantum;

        if (level == 0)
            quantum = 2;

        else if (level == 1)
            quantum = 4;

        else
            quantum = current.remaining;

        int execute =
            min(quantum, current.remaining);

        int start = time;

        time += execute;

        current.remaining -= execute;

        // Gantt block
        addBlock(
            "PROCESS",
            current.pid,
            start,
            time
        );

        checkStarvation(p, time, starvationThreshold);

        // Add processes that arrived
        // during execution
        for (int i = 0; i < n; i++) {

            if (!added[i] &&
                p[i].at <= time) {

                int newLevel =
                    getMLQLevel(p[i]);

                q[newLevel].push(i);
                added[i] = true;
            }
        }

        if (current.remaining == 0) {

            current.ct = time;

            current.tat =
                current.ct - current.at;

            current.wt =
                current.tat - current.bt;

            completed++;

        }
        else {

            // MLQ:
            // same queue mein wapas
            q[level].push(idx);
        }

        previousPid = current.pid;
    }
}

void runMLFQ(vector<Process>& p, int csTime, int agingThreshold) {
    int n = p.size();

    int time = 0;
    int completed = 0;

    const int Q0_QUANTUM = 2;
    const int Q1_QUANTUM = 4;

    queue<int> q0;
    queue<int> q1;
    queue<int> q2;

    vector<bool> added(n, false);

    // Current queue of every process
    vector<int> level(n);

    // Last time process got CPU
    vector<int> lastRun(n, -1);

    // Initial queue based on priority
    for (int i = 0; i < n; i++) {
        level[i] = getMLQLevel(p[i]);
    }

    int previousPid = -1;

    while (completed < n) {

        // ==========================================
        // ADD NEW ARRIVALS
        // ==========================================

        for (int i = 0; i < n; i++) {

            if (!added[i] &&
                p[i].at <= time) {

                if (level[i] == 0)
                    q0.push(i);

                else if (level[i] == 1)
                    q1.push(i);

                else
                    q2.push(i);

                added[i] = true;

                // Start waiting timer
                lastRun[i] = time;
            }
        }

        // ==========================================
        // AGING
        // ==========================================

        for (int i = 0; i < n; i++) {

            if (!added[i])
                continue;

            if (p[i].remaining <= 0)
                continue;

            int waitingTime =
                time - lastRun[i];

            if (waitingTime >= agingThreshold) {

                // Q2 → Q1
                if (level[i] == 2) {

                    level[i] = 1;
                    q1.push(i);

                    lastRun[i] = time;
                }

                // Q1 → Q0
                else if (level[i] == 1) {

                    level[i] = 0;
                    q0.push(i);

                    lastRun[i] = time;
                }
            }
        }

        // ==========================================
        // SELECT PROCESS
        // ==========================================

        int idx = -1;

        while (!q0.empty()) {

            int temp = q0.front();
            q0.pop();

            if (level[temp] == 0 &&
                p[temp].remaining > 0) {

                idx = temp;
                break;
            }
        }

        if (idx == -1) {

            while (!q1.empty()) {

                int temp = q1.front();
                q1.pop();

                if (level[temp] == 1 &&
                    p[temp].remaining > 0) {

                    idx = temp;
                    break;
                }
            }
        }

        if (idx == -1) {

            while (!q2.empty()) {

                int temp = q2.front();
                q2.pop();

                if (level[temp] == 2 &&
                    p[temp].remaining > 0) {

                    idx = temp;
                    break;
                }
            }
        }

        // ==========================================
        // CPU IDLE
        // ==========================================

        if (idx == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {

                if (!added[i]) {

                    nextArrival =
                        min(
                            nextArrival,
                            p[i].at
                        );
                }
            }

            if (nextArrival != INT_MAX) {

                addIdle(
                    time,
                    nextArrival
                );

                time = nextArrival;
            }

            previousPid = -1;

            continue;
        }

        Process &current = p[idx];

        // ==========================================
        // CONTEXT SWITCH
        // ==========================================

        if (previousPid != -1 &&
            previousPid != current.pid &&
            csTime > 0) {

            addContextSwitch(
                time,
                csTime
            );

            time += csTime;

            contextSwitches++;
        }

        // ==========================================
        // RESPONSE TIME
        // ==========================================

        if (current.rt == -1) {

            current.rt =
                time - current.at;
        }

        // ==========================================
        // SELECT QUANTUM
        // ==========================================

        int quantum;

        if (level[idx] == 0)
            quantum = Q0_QUANTUM;

        else if (level[idx] == 1)
            quantum = Q1_QUANTUM;

        else
            quantum = current.remaining;

        int execute =
            min(
                quantum,
                current.remaining
            );

        int start = time;

        time += execute;

        current.remaining -= execute;

        checkStarvation(p, time, agingThreshold);

        // ==========================================
        // GANTT
        // ==========================================

        addBlock(
            "PROCESS",
            current.pid,
            start,
            time
        );

        // ==========================================
        // PROCESS FINISHED
        // ==========================================

        if (current.remaining == 0) {

            current.ct = time;

            current.tat =
                current.ct -
                current.at;

            current.wt =
                current.tat -
                current.bt;

            completed++;
        }

        // ==========================================
        // PROCESS NOT FINISHED
        // ==========================================

        else {

            // Update last CPU time
            lastRun[idx] = time;

            // Q0 → Q1
            if (level[idx] == 0) {

                level[idx] = 1;
                q1.push(idx);
            }

            // Q1 → Q2
            else if (level[idx] == 1) {

                level[idx] = 2;
                q2.push(idx);
            }

            // Q2 stays in Q2
            else {

                q2.push(idx);
            }
        }

        // ==========================================
        // ADD NEW ARRIVALS AFTER EXECUTION
        // ==========================================

        for (int i = 0; i < n; i++) {

            if (!added[i] &&
                p[i].at <= time) {

                if (level[i] == 0)
                    q0.push(i);

                else if (level[i] == 1)
                    q1.push(i);

                else
                    q2.push(i);

                added[i] = true;

                lastRun[i] = time;
            }
        }

        previousPid =
            current.pid;
    }
}

void calculateMetrics() {

    for (auto &p : processes) {

        p.tat = p.ct - p.at;
        p.wt = p.tat - p.bt;
    }
}

struct CompareSJF {

    bool operator()(const Process &a,
                    const Process &b) const {

        if (a.bt != b.bt)
            return a.bt > b.bt;

        if (a.at != b.at)
            return a.at > b.at;

        return a.pid > b.pid;
    }
};

struct ComparePriority {

    bool operator()(const Process &a,
                    const Process &b) const {

        // Smaller priority number = higher priority

        if (a.priority != b.priority)
            return a.priority > b.priority;

        if (a.at != b.at)
            return a.at > b.at;

        return a.pid > b.pid;
    }
};

void FCFS(int csTime) {

    sort(processes.begin(),
         processes.end(),
         [](const Process &a, const Process &b) {

        if (a.at != b.at)
            return a.at < b.at;

        return a.pid < b.pid;
    });

    int time = 0;
    int previousPid = -1;

    for (auto &p : processes) {

        // CPU idle
        if (time < p.at) {

            addIdle(time, p.at);
            time = p.at;
            previousPid = -1;
        }

        // Context switch
        if (previousPid != -1 &&
            previousPid != p.pid &&
            csTime > 0) {

            addContextSwitch(time, csTime);
            time += csTime;

            contextSwitches++;
        }

        p.rt = time - p.at;

        int start = time;

        time += p.bt;

        p.ct = time;

        addBlock(
            "PROCESS",
            p.pid,
            start,
            time
        );

        previousPid = p.pid;
    }
}

void SJF(int csTime) {

    sort(processes.begin(),
         processes.end(),
         [](const Process &a, const Process &b) {

        if (a.at != b.at)
            return a.at < b.at;

        return a.pid < b.pid;
    });

    priority_queue<
        Process,
        vector<Process>,
        CompareSJF
    > pq;

    int n = processes.size();

    int index = 0;
    int time = 0;
    int previousPid = -1;

    while (index < n || !pq.empty()) {

        // If no process is ready
        if (pq.empty()) {

            if (time < processes[index].at) {

                addIdle(
                    time,
                    processes[index].at
                );

                time = processes[index].at;
                previousPid = -1;
            }

            while (
                index < n &&
                processes[index].at <= time
            ) {

                pq.push(processes[index]);
                index++;
            }
        }

        while (
            index < n &&
            processes[index].at <= time
        ) {

            pq.push(processes[index]);
            index++;
        }

        Process current = pq.top();
        pq.pop();

        if (previousPid != -1 &&
            previousPid != current.pid &&
            csTime > 0) {

            addContextSwitch(time, csTime);
            time += csTime;

            contextSwitches++;
        }

        current.rt = time - current.at;

        int start = time;

        time += current.bt;

        current.ct = time;

        addBlock(
            "PROCESS",
            current.pid,
            start,
            time
        );

        for (auto &p : processes) {

            if (p.pid == current.pid) {

                p.rt = current.rt;
                p.ct = current.ct;

                break;
            }
        }

        previousPid = current.pid;
    }
}

void PriorityScheduling(int csTime) {

    sort(processes.begin(),
         processes.end(),
         [](const Process &a, const Process &b) {

        if (a.at != b.at)
            return a.at < b.at;

        return a.pid < b.pid;
    });

    priority_queue<
        Process,
        vector<Process>,
        ComparePriority
    > pq;

    int n = processes.size();

    int index = 0;
    int time = 0;
    int previousPid = -1;

    while (index < n || !pq.empty()) {

        if (pq.empty()) {

            if (time < processes[index].at) {

                addIdle(
                    time,
                    processes[index].at
                );

                time = processes[index].at;
                previousPid = -1;
            }

            while (
                index < n &&
                processes[index].at <= time
            ) {

                pq.push(processes[index]);
                index++;
            }
        }

        while (
            index < n &&
            processes[index].at <= time
        ) {

            pq.push(processes[index]);
            index++;
        }

        Process current = pq.top();
        pq.pop();

        if (previousPid != -1 &&
            previousPid != current.pid &&
            csTime > 0) {

            addContextSwitch(time, csTime);
            time += csTime;

            contextSwitches++;
        }

        current.rt = time - current.at;

        int start = time;

        time += current.bt;

        current.ct = time;

        addBlock(
            "PROCESS",
            current.pid,
            start,
            time
        );

        for (auto &p : processes) {

            if (p.pid == current.pid) {

                p.rt = current.rt;
                p.ct = current.ct;

                break;
            }
        }

        previousPid = current.pid;
    }
}

void SRTF(int csTime) {

    int n = processes.size();

    int time = 0;
    int completed = 0;

    int previousPid = -1;

    while (completed < n) {

        int idx = -1;

        // Find process with minimum remaining time
        for (int i = 0; i < n; i++) {

            if (processes[i].at <= time &&
                processes[i].remaining > 0) {

                if (idx == -1 ||
                    processes[i].remaining <
                    processes[idx].remaining) {

                    idx = i;
                }
                else if (
                    processes[i].remaining ==
                    processes[idx].remaining
                ) {

                    if (processes[i].at <
                        processes[idx].at) {

                        idx = i;
                    }
                    else if (
                        processes[i].at ==
                        processes[idx].at &&
                        processes[i].pid <
                        processes[idx].pid
                    ) {

                        idx = i;
                    }
                }
            }
        }

        // ---------------------------
        // CPU IDLE
        // ---------------------------

        if (idx == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {

                if (processes[i].remaining > 0) {

                    nextArrival =
                        min(
                            nextArrival,
                            processes[i].at
                        );
                }
            }

            if (nextArrival != INT_MAX &&
                time < nextArrival) {

                addIdle(
                    time,
                    nextArrival
                );

                time = nextArrival;

                previousPid = -1;
            }

            continue;
        }

        Process &current = processes[idx];

        // ---------------------------
        // CONTEXT SWITCH
        // ---------------------------

        if (previousPid != -1 && previousPid != current.pid && csTime > 0) {

            addContextSwitch(time, csTime);

            time += csTime;

            contextSwitches++;

            // Re-evaluate after context switch
            previousPid = -1;

            continue;
        }

        // ---------------------------
        // RESPONSE TIME
        // ---------------------------

        if (current.rt == -1) {

            current.rt =
                time - current.at;
        }

        // ---------------------------
        // EXECUTE 1 TIME UNIT
        // ---------------------------

        int start = time;

        time++;

        current.remaining--;

        addBlock(
            "PROCESS",
            current.pid,
            start,
            time
        );

        // ---------------------------
        // PROCESS COMPLETED
        // ---------------------------

        if (current.remaining == 0) {

            current.ct = time;

            current.tat =
                current.ct -
                current.at;

            current.wt =
                current.tat -
                current.bt;

            completed++;
        }

        previousPid = current.pid;
    }
}

void PreemptivePriority(int csTime) {

    int n = processes.size();

    int time = 0;
    int completed = 0;

    int previousPid = -1;

    while (completed < n) {

        int idx = -1;

        // Smaller priority number = higher priority
        for (int i = 0; i < n; i++) {

            if (processes[i].at <= time &&
                processes[i].remaining > 0) {

                if (idx == -1 ||
                    processes[i].priority <
                    processes[idx].priority) {

                    idx = i;
                }
                else if (
                    processes[i].priority ==
                    processes[idx].priority
                ) {

                    if (processes[i].at <
                        processes[idx].at) {

                        idx = i;
                    }
                    else if (
                        processes[i].at ==
                        processes[idx].at &&
                        processes[i].pid <
                        processes[idx].pid
                    ) {

                        idx = i;
                    }
                }
            }
        }

        // ---------------------------
        // CPU IDLE
        // ---------------------------

        if (idx == -1) {

            int nextArrival = INT_MAX;

            for (int i = 0; i < n; i++) {

                if (processes[i].remaining > 0) {

                    nextArrival =
                        min(
                            nextArrival,
                            processes[i].at
                        );
                }
            }

            if (nextArrival != INT_MAX &&
                time < nextArrival) {

                addIdle(
                    time,
                    nextArrival
                );

                time = nextArrival;

                previousPid = -1;
            }

            continue;
        }

        Process &current = processes[idx];

        // ---------------------------
        // CONTEXT SWITCH
        // ---------------------------

        if (previousPid != -1 &&
            previousPid != current.pid &&
            csTime > 0) {

            addContextSwitch(
                time,
                csTime
            );

            time += csTime;

            contextSwitches++;

            // Re-evaluate after CS
            previousPid = -1;

            continue;
        }

        // ---------------------------
        // RESPONSE TIME
        // ---------------------------

        if (current.rt == -1) {

            current.rt =
                time - current.at;
        }

        // ---------------------------
        // EXECUTE 1 TIME UNIT
        // ---------------------------

        int start = time;

        time++;

        current.remaining--;

        addBlock(
            "PROCESS",
            current.pid,
            start,
            time
        );

        // ---------------------------
        // PROCESS COMPLETED
        // ---------------------------

        if (current.remaining == 0) {

            current.ct = time;

            current.tat =
                current.ct -
                current.at;

            current.wt =
                current.tat -
                current.bt;

            completed++;
        }

        previousPid = current.pid;
    }
}

void RoundRobin(
    int quantum,
    int csTime
) {

    sort(processes.begin(),
         processes.end(),
         [](const Process &a, const Process &b) {

        if (a.at != b.at)
            return a.at < b.at;

        return a.pid < b.pid;
    });

    int n = processes.size();

    queue<int> q;

    vector<int> remaining(n);

    for (int i = 0; i < n; i++) {

        remaining[i] = processes[i].bt;
    }

    int index = 0;
    int completed = 0;
    int time = 0;

    int previousPid = -1;

    while (completed < n) {

        // CPU idle
        if (q.empty()) {

            if (index < n &&
                time < processes[index].at) {

                addIdle(
                    time,
                    processes[index].at
                );

                time = processes[index].at;
                previousPid = -1;
            }

            while (
                index < n &&
                processes[index].at <= time
            ) {

                q.push(index);
                index++;
            }
        }

        if (q.empty())
            continue;

        int idx = q.front();
        q.pop();

        Process &p = processes[idx];

        // Context switch
        if (previousPid != -1 &&
            previousPid != p.pid &&
            csTime > 0) {

            addContextSwitch(time, csTime);

            time += csTime;

            contextSwitches++;
        }

        // Response time
        if (p.rt == -1) {

            p.rt = time - p.at;
        }

        int start = time;

        int execution = min(
            quantum,
            remaining[idx]
        );

        time += execution;

        remaining[idx] -= execution;

        addBlock(
            "PROCESS",
            p.pid,
            start,
            time
        );

        // New processes arrive during execution
        while (
            index < n &&
            processes[index].at <= time
        ) {

            q.push(index);
            index++;
        }

        if (remaining[idx] > 0) {

            q.push(idx);

        } else {

            p.ct = time;
            completed++;
        }

        previousPid = p.pid;
    }
}

void printJSON(
    string algorithm,
    int quantum,
    int csTime,
    int starvationThreshold
) {

    calculateMetrics();

    // Final starvation check
    for (auto &p : processes) {

        p.starved = (p.wt >= starvationThreshold);
    }

    double avgWT = 0;
    double avgTAT = 0;
    double avgRT = 0;

    int n = processes.size();

    int totalBurst = 0;
    int totalTime = 0;

    for (auto &p : processes) {

        avgWT += p.wt;
        avgTAT += p.tat;
        avgRT += p.rt;

        totalBurst += p.bt;
        totalTime = max(totalTime, p.ct);
    }

    double cpuUtilization = 0;

    if (totalTime > 0) {

        cpuUtilization =
            (double) totalBurst /
            totalTime * 100.0;
    }

    cout << fixed << setprecision(2);

    cout << "{";

    cout << "\"algorithm\":\""
         << algorithm
         << "\",";

    cout << "\"quantum\":"
         << quantum
         << ",";

    cout << "\"contextSwitchTime\":"
         << csTime
         << ",";

    cout << "\"contextSwitches\":"
         << contextSwitches
         << ",";

    cout << "\"cpuIdleTime\":"
         << totalTime -
            totalBurst -
            contextSwitchTime
         << ",";

    cout << "\"cpuUtilization\":"
         << cpuUtilization
         << ",";

    cout << "\"avgWaitingTime\":"
         << avgWT / n
         << ",";

    cout << "\"avgTurnaroundTime\":"
         << avgTAT / n
         << ",";

    cout << "\"avgResponseTime\":"
         << avgRT / n
         << ",";

    cout << "\"processes\":[";

    for (int i = 0; i < n; i++) {

        auto &p = processes[i];

        if (i)
            cout << ",";

        cout << "{";

        cout << "\"pid\":" << p.pid << ",";

        cout << "\"arrivalTime\":" << p.at << ",";

        cout << "\"burstTime\":" << p.bt << ",";

        cout << "\"priority\":" << p.priority << ",";

        cout << "\"completionTime\":" << p.ct << ",";

        cout << "\"turnaroundTime\":"<< p.tat << ",";

        cout << "\"waitingTime\":"<< p.wt << ",";

        cout << "\"responseTime\":" << p.rt << ",";

        cout << "\"starved\":" << (p.starved ? "true" : "false");

        cout << "}";
    }

    cout << "],";

    cout << "\"gantt\":[";

    for (int i = 0;
         i < (int)gantt.size();
         i++) {

        if (i)
            cout << ",";

        auto &g = gantt[i];

        cout << "{";

        cout << "\"type\":\""
             << g.type
             << "\",";

        cout << "\"pid\":"
             << g.pid
             << ",";

        cout << "\"start\":"
             << g.start
             << ",";

        cout << "\"end\":"
             << g.end;

        cout << "}";
    }

    cout << "]";

    cout << "}";

    cout << endl;
}

int main() {

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string algorithm;

    getline(cin, algorithm);

    int quantum;
    int csTime;
    int starvationThreshold;
    int n;

    cin >> quantum;
    cin >> csTime;
    cin >> starvationThreshold;
    cin >> n;

    processes.resize(n);

    for (int i = 0; i < n; i++) {

        cin >>
            processes[i].pid >>
            processes[i].at >>
            processes[i].bt >>
            processes[i].priority;

        processes[i].remaining = processes[i].bt;
    }

    if (algorithm == "FCFS") {

        FCFS(csTime);

    }
    else if (algorithm == "SJF") {

        SJF(csTime);

    }

    else if (algorithm == "SRTF") {

        SRTF(csTime);
    }

    else if (algorithm == "PRIORITY") {

        PriorityScheduling(csTime);

    }

    else if (algorithm == "PREEMPTIVE_PRIORITY") { 

        PreemptivePriority(csTime);

    }

    else if (algorithm == "RR") {

        RoundRobin(quantum, csTime);

    }

    else if (algorithm == "MLQ") {

        runMLQ(processes, csTime, starvationThreshold);
    }

    else if (algorithm == "MLFQ") {

        runMLFQ(processes, csTime, starvationThreshold);
    }

    else {

        cerr << "Invalid algorithm";
        return 1;
    }

    printJSON(
        algorithm,
        quantum,
        csTime,
        starvationThreshold
    );

    return 0;
}