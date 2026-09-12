let processes = [];

const arrivalInput = document.getElementById("arrival");

const burstInput = document.getElementById("burst");

const priorityInput = document.getElementById("priority");

const processTable = document.getElementById("processTable");

let dashboardTimer = null;

function showProcessStates(processes, currentTime, currentBlock) {

    const container = document.getElementById("processStates");

    if (!container)
        return;

    container.innerHTML = "";

    processes.sort((a,b)=>a.pid-b.pid);

    processes.forEach(p => {

        let state = "READY";

        // --------------------------------
        // Process has not arrived yet
        // --------------------------------

        if (currentTime < p.arrivalTime) {

            state = "READY";

        }

        // --------------------------------
        // Process already completed
        // --------------------------------

        else if (
            currentTime >= p.completionTime
        ) {

            state = "COMPLETED";

        }

        // --------------------------------
        // Currently running
        // --------------------------------

        else if (
            currentBlock &&
            currentBlock.type === "PROCESS" &&
            currentBlock.pid === p.pid
        ) {

            state = "RUNNING";

        }

        // --------------------------------
        // Arrived but waiting for CPU
        // --------------------------------

        else {

            state = "READY";
        }


        const card =
            document.createElement("div");

        card.className =
            `process-card ${state.toLowerCase()}`;


        card.innerHTML = `

            <div class="process-card-header">

                <div class="process-pid">
                    P${p.pid}
                </div>

                <div class="process-state-badge">
                    ${state}
                </div>

            </div>

            <div class="process-details">

                <div>
                    <span>Arrival</span>
                    <strong>${p.arrivalTime}</strong>
                </div>

                <div>
                    <span>Burst</span>
                    <strong>${p.burstTime}</strong>
                </div>

                <div>
                    <span>Priority</span>
                    <strong>${p.priority}</strong>
                </div>

            </div>
        `;

        container.appendChild(card);
    });
}

function startLiveDashboard(data) {

    // Previous simulation stop karo
    if (dashboardTimer !== null) {
        clearInterval(dashboardTimer);
        dashboardTimer = null;
    }

    document.getElementById("currentTime").textContent = "0";
    document.getElementById("cpuState").textContent = "IDLE";
    document.getElementById("cpuProcess").textContent = "No Process";
    document.getElementById("liveUtilization").textContent = "0%";
    document.getElementById("liveContextSwitches").textContent = "0";

    let currentTime = 0;

    const gantt = data.gantt;

    if (!gantt || gantt.length === 0) {
        return;
    }

    showProcessStates(data.processes, 0, null);

    const maxTime =
        gantt[gantt.length - 1].end;

    dashboardTimer = setInterval(() => {

        let currentBlock = null;

        // ---------------------------
        // Find current CPU block
        // ---------------------------

        for (const block of gantt) {

            if (
                currentTime >= block.start &&
                currentTime < block.end
            ) {
                currentBlock = block;
                break;
            }
        }

        showProcessStates(data.processes, currentTime, currentBlock);

        // ---------------------------
        // Current Time
        // ---------------------------

        document.getElementById(
            "currentTime"
        ).textContent = currentTime;


        // ---------------------------
        // CPU State
        // ---------------------------

        if (currentBlock) {

            if (currentBlock.type === "IDLE") {

                document.getElementById(
                    "cpuState"
                ).textContent = "IDLE";

                document.getElementById(
                    "cpuProcess"
                ).textContent = "No Process";

            }
            else if (currentBlock.type === "CS") {

                document.getElementById(
                    "cpuState"
                ).textContent =
                    "CONTEXT SWITCH";

                document.getElementById(
                    "cpuProcess"
                ).textContent =
                    "Switching...";

            }
            else {

                document.getElementById(
                    "cpuState"
                ).textContent =
                    "RUNNING";

                document.getElementById(
                    "cpuProcess"
                ).textContent =
                    "P" + currentBlock.pid;
            }
        }


        // ---------------------------
        // LIVE CPU UTILIZATION
        // ---------------------------

        let busyTime = 0;

        for (const block of gantt) {

            if (block.start >= currentTime) {
                continue;
            }

            if (block.type !== "PROCESS") {
                continue;
            }

            const start =
                block.start;

            const end =
                Math.min(
                    block.end,
                    currentTime
                );

            if (end > start) {
                busyTime += end - start;
            }
        }

        let utilization = 0;

        if (currentTime > 0) {

            utilization =
                (busyTime / currentTime) * 100;
        }

        document.getElementById(
            "liveUtilization"
        ).textContent =
            utilization.toFixed(2) + "%";


        // ---------------------------
        // LIVE CONTEXT SWITCHES
        // ---------------------------

        let switches = 0;

        for (const block of gantt) {

            if (
                block.type === "CS" &&
                block.start < currentTime
            ) {
                switches++;
            }
        }

        document.getElementById(
            "liveContextSwitches"
        ).textContent = switches;


        // ---------------------------
        // Next Time
        // ---------------------------

        currentTime++;

        // ---------------------------
        // Simulation Finished
        // ---------------------------

        if (currentTime > maxTime) {

            clearInterval(dashboardTimer);
            dashboardTimer = null;

            document.getElementById(
                "currentTime"
            ).textContent = maxTime;

            document.getElementById(
                "cpuState"
            ).textContent = "FINISHED";

            document.getElementById(
                "cpuProcess"
            ).textContent =
                "Simulation Complete";

            // Final utilization
            document.getElementById(
                "liveUtilization"
            ).textContent =
                data.cpuUtilization.toFixed(2) + "%";

            document.getElementById(
                "liveContextSwitches"
            ).textContent =
                data.contextSwitches;

            showProcessStates(data.processes, maxTime, null);
        }

    }, 700);
}

// ---------------------------
// Render Processes
// ---------------------------

function renderProcesses() {

    processTable.innerHTML = "";

    processes.forEach((p, index) => {

        const row =
            document.createElement("tr");

        row.innerHTML = `
            <td>P${p.pid}</td>
            <td>${p.arrivalTime}</td>
            <td>${p.burstTime}</td>
            <td>${p.priority}</td>

            <td>
                <button
                    class="delete-btn"
                    onclick="deleteProcess(${index})"
                >
                    Delete
                </button>
            </td>
        `;

        processTable.appendChild(row);
    });
}


// ---------------------------
// Add Process
// ---------------------------

document
    .getElementById("addProcess")
    .addEventListener("click", () => {

        const at =
            Number(arrivalInput.value);

        const bt =
            Number(burstInput.value);

        const priority =
            Number(priorityInput.value);

        if (at < 0 || bt <= 0 || priority <= 0) {

            alert(
                "Please enter valid process values."
            );

            return;
        }

        processes.push({

            pid: processes.length + 1,

            arrivalTime: at,

            burstTime: bt,

            priority: priority
        });

        renderProcesses();

        arrivalInput.value = "";
        burstInput.value = "";
        priorityInput.value = "";
    });


// ---------------------------
// Delete Process
// ---------------------------

function deleteProcess(index) {

    processes.splice(index, 1);

    processes.forEach((p, i) => {

        p.pid = i + 1;
    });

    renderProcesses();
}


// ---------------------------
// Run Simulation
// ---------------------------

document
    .getElementById("simulate")
    .addEventListener("click", async () => {

        if (processes.length === 0) {

            alert(
                "Add at least one process."
            );

            return;
        }

        const algorithm =
            document.getElementById(
                "algorithm"
            ).value;

        const quantum =
            Number(
                document.getElementById(
                    "quantum"
                ).value
            );

        const contextSwitch =
            Number(
                document.getElementById(
                    "contextSwitch"
                ).value
            );

        const starvationThreshold = 
            Number(
                document.getElementById(
                    "starvationThreshold"
                ).value
            );

        if (starvationThreshold <= 0) {
            alert("Starvation threshold must be greater than 0.");
            return;
        }

        if (
            algorithm === "RR" &&
            quantum <= 0
        ) {

            alert(
                "Time quantum must be greater than 0."
            );

            return;
        }

        try {

            const response =
                await fetch(
                    "/api/schedule",
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body: JSON.stringify({

                            algorithm,

                            quantum,

                            contextSwitch,

                            starvationThreshold,

                            processes
                        })
                    }
                );

            const result =
                await response.json();

            if (!response.ok) {

                throw new Error(
                    result.error
                );
            }

            showResults(result, starvationThreshold);

            document.getElementById("results").scrollIntoView({
                behavior: "smooth", 
                block: "start"
            });

        } catch (error) {

            alert(error.message);
        }
    });


// ---------------------------
// Show Results
// ---------------------------

function showResults(result, starvationThreshold) {

    document
        .getElementById("results")
        .classList.remove("hidden");

    const metrics =
        document.getElementById(
            "metrics"
        );

    metrics.innerHTML = `

        <div class="metric waiting">
            Avg Waiting Time
            <strong>
                ${result.avgWaitingTime}
            </strong>
        </div>

        <div class="metric turnaround">
            Avg Turnaround Time
            <strong>
                ${result.avgTurnaroundTime}
            </strong>
        </div>

        <div class="metric response">
            Avg Response Time
            <strong>
                ${result.avgResponseTime}
            </strong>
        </div>

        <div class="metric utilization">
            CPU Utilization
            <strong>
                ${result.cpuUtilization}%
            </strong>
        </div>

        <div class="metric idle">
            CPU Idle Time
            <strong>
                ${result.cpuIdleTime}
            </strong>
        </div>

        <div class="metric switches">
            Context Switches
            <strong>
                ${result.contextSwitches}
            </strong>
        </div>

    `;

    renderGantt(result.gantt);

    startLiveDashboard(result);

    renderResultTable(result.processes, starvationThreshold);

}


// ---------------------------
// Gantt Chart
// ---------------------------

function renderGantt(gantt) {

    const container =
        document.getElementById(
            "gantt"
        );

    container.innerHTML = "";

    gantt.forEach(block => {

        const div =
            document.createElement("div");

        div.classList.add(
            "gantt-block"
        );

        if (block.type === "PROCESS") {

            div.classList.add(
                "process-block"
            );

            div.innerHTML = `
                P${block.pid}
                <div class="time">
                    ${block.start} - ${block.end}
                </div>
            `;

        }
        else if (block.type === "IDLE") {

            div.classList.add(
                "idle-block"
            );

            div.innerHTML = `
                IDLE
                <div class="time">
                    ${block.start} - ${block.end}
                </div>
            `;

        }
        else {

            div.classList.add(
                "cs-block"
            );

            div.innerHTML = `
                CS
                <div class="time">
                    ${block.start} - ${block.end}
                </div>
            `;
        }

        container.appendChild(div);
    });
}


// ---------------------------
// Result Table
// ---------------------------

function renderResultTable(data, starvationThreshold) {

    const table = document.getElementById("resultTable");

    table.innerHTML = "";

    data.forEach(p => {

        const row = document.createElement("tr");

        // Starvation detection
        const starved = p.starved;  

        row.innerHTML = `

            <td>P${p.pid}</td>

            <td>${p.arrivalTime}</td>

            <td>${p.burstTime}</td>

            <td>${p.priority}</td>

            <td>${p.completionTime}</td>

            <td>${p.turnaroundTime}</td>

            <td>${p.waitingTime}</td>

            <td>${p.responseTime}</td>

            <td class="${starved ? "starved" : ""}">
                ${
                    starved
                        ? "⚠ Starvation"
                        : "Normal"
                }
            </td>

        `;

        table.appendChild(row);
    });
}


// ---------------------------
// Compare Algorithms
// ---------------------------

document
    .getElementById("compare")
    .addEventListener("click", async () => {

        if (processes.length === 0) {

            alert(
                "Add processes first."
            );

            return;
        }

        const quantum =
            Number(
                document.getElementById("quantum").value
            );

        const contextSwitch =
            Number(
                document.getElementById("contextSwitch").value
            );

        const starvationThreshold =
            Number(
                document.getElementById("starvationThreshold").value
            );

        try {

            const response =
                await fetch(
                    "/api/compare",
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body: JSON.stringify({

                            quantum,

                            contextSwitch,

                            starvationThreshold,

                            processes
                        })
                    }
                );

            const results =
                await response.json();

            if (!response.ok) {

                throw new Error(
                    results.error
                );
            }

            showComparison(results);

            document.getElementById("comparison").scrollIntoView({
                behavior: "smooth", block: "start"
            });

        } catch (error) {

            alert(error.message);
        }
    });

// ---------------------------
// Algorithm Performance Chart
// ---------------------------

function showPerformanceChart(results) {

    const comparison =
        document.getElementById("comparison");

    // Existing chart remove karo
    const oldChart =
        document.getElementById("performanceChartSection");

    if (oldChart) {
        oldChart.remove();
    }

    // ---------------------------
    // Create chart section
    // ---------------------------

    const section =
        document.createElement("div");

    section.id =
        "performanceChartSection";

    section.className =
        "performance-chart-section";

    section.innerHTML = `

        <h3>Algorithm Performance</h3>

        <div class="chart-buttons">

            <button
                class="chart-btn active"
                data-metric="waiting"
            >
                Waiting Time
            </button>

            <button
                class="chart-btn"
                data-metric="turnaround"
            >
                Turnaround Time
            </button>

            <button
                class="chart-btn"
                data-metric="response"
            >
                Response Time
            </button>

            <button
                class="chart-btn"
                data-metric="utilization"
            >
                CPU Utilization
            </button>

            <button
                class="chart-btn"
                data-metric="switches"
            >
                Context Switches
            </button>

        </div>

        <div
            id="performanceChart"
            class="performance-chart"
        ></div>
    `;

    comparison.appendChild(section);


    // ---------------------------
    // Chart rendering function
    // ---------------------------

    function renderChart(metric) {

        const chart =
            document.getElementById(
                "performanceChart"
            );

        chart.innerHTML = "";


        let title = "";

        let getValue;


        if (metric === "waiting") {

            title = "Average Waiting Time";

            getValue =
                result =>
                    Number(
                        result.avgWaitingTime
                    );
        }

        else if (metric === "turnaround") {

            title = "Average Turnaround Time";

            getValue =
                result =>
                    Number(
                        result.avgTurnaroundTime
                    );
        }

        else if (metric === "response") {

            title = "Average Response Time";

            getValue =
                result =>
                    Number(
                        result.avgResponseTime
                    );
        }

        else if (metric === "utilization") {

            title = "CPU Utilization";

            getValue =
                result =>
                    Number(
                        result.cpuUtilization
                    );
        }

        else {

            title = "Context Switches";

            getValue =
                result =>
                    Number(
                        result.contextSwitches
                    );
        }


        // ---------------------------
        // Find best value
        // ---------------------------

        const values =
            results.map(result =>
                getValue(result)
            );

        let bestValue;

        if (metric === "utilization") {

            bestValue =
                Math.max(...values);

        } else {

            bestValue =
                Math.min(...values);
        }


        // ---------------------------
        // Chart title
        // ---------------------------

        const heading =
            document.createElement("div");

        heading.className =
            "chart-title";

        heading.textContent =
            title;

        chart.appendChild(heading);


        // ---------------------------
        // Bars
        // ---------------------------

        const bars =
            document.createElement("div");

        bars.className =
            "chart-bars";


        const maxValue =
            Math.max(...values, 1);


        results.forEach(result => {

            const value =
                getValue(result);

            const algorithmName =
                result.algorithm ===
                "PREEMPTIVE_PRIORITY"

                    ? "Preemptive Priority"

                    : result.algorithm;


            const barItem =
                document.createElement("div");

            barItem.className =
                "chart-item";


            const percentage =
                (value / maxValue) * 100;


            const isBest =
                value === bestValue;


            barItem.innerHTML = `

                <div class="chart-label">
                    ${algorithmName}
                </div>

                <div class="bar-wrapper">

                    <div
                        class="chart-bar ${
                            isBest
                                ? "chart-best"
                                : ""
                        }"
                        style="
                            width: ${percentage}%;
                        "
                    >

                        <span>
                            ${value}${
                                metric ===
                                "utilization"
                                    ? "%"
                                    : ""
                            }
                        </span>

                    </div>

                </div>

                ${
                    isBest
                        ? `<span class="chart-trophy">
                                🏆 Best
                           </span>`
                        : ""
                }

            `;

            bars.appendChild(barItem);
        });


        chart.appendChild(bars);
    }


    // ---------------------------
    // Button events
    // ---------------------------

    const buttons =
        section.querySelectorAll(
            ".chart-btn"
        );


    buttons.forEach(button => {

        button.addEventListener(
            "click",
            () => {

                buttons.forEach(btn =>
                    btn.classList.remove(
                        "active"
                    )
                );

                button.classList.add(
                    "active"
                );

                renderChart(
                    button.dataset.metric
                );
            }
        );

    });


    // First chart
    renderChart("waiting");
}

// ---------------------------
// Comparison Table
// ---------------------------

function showComparison(results) {

    document
        .getElementById("comparison")
        .classList.remove("hidden");

    const table =
        document.getElementById("comparisonTable");

    table.innerHTML = "";

    // Find best values
    const bestWT = Math.min(
        ...results.map(r => r.avgWaitingTime)
    );

    const bestTAT = Math.min(
        ...results.map(r => r.avgTurnaroundTime)
    );

    const bestRT = Math.min(
        ...results.map(r => r.avgResponseTime)
    );

    const bestUtilization = Math.max(
        ...results.map(r => r.cpuUtilization)
    );

    const bestContextSwitches = Math.min(
        ...results.map(r => r.contextSwitches)
    );


    results.forEach(result => {

        const row = document.createElement("tr");

        const algorithmName =
            result.algorithm === "PREEMPTIVE_PRIORITY"
                ? "Preemptive Priority"
                : result.algorithm;

        row.innerHTML = `

            <td>
                ${algorithmName}
            </td>

            <td class="${
                result.avgWaitingTime === bestWT
                    ? "best-metric"
                    : ""
            }">
                ${result.avgWaitingTime}
                ${
                    result.avgWaitingTime === bestWT
                        ? " 🏆"
                        : ""
                }
            </td>

            <td class="${
                result.avgTurnaroundTime === bestTAT
                    ? "best-metric"
                    : ""
            }">
                ${result.avgTurnaroundTime}
                ${
                    result.avgTurnaroundTime === bestTAT
                        ? " 🏆"
                        : ""
                }
            </td>

            <td class="${
                result.avgResponseTime === bestRT
                    ? "best-metric"
                    : ""
            }">
                ${result.avgResponseTime}
                ${
                    result.avgResponseTime === bestRT
                        ? " 🏆"
                        : ""
                }
            </td>

            <td class="${
                result.cpuUtilization === bestUtilization
                    ? "best-metric"
                    : ""
            }">
                ${result.cpuUtilization}%
                ${
                    result.cpuUtilization === bestUtilization
                        ? " 🏆"
                        : ""
                }
            </td>

            <td class="${
                result.contextSwitches === bestContextSwitches
                    ? "best-metric"
                    : ""
            }">
                ${result.contextSwitches}
                ${
                    result.contextSwitches === bestContextSwitches
                        ? " 🏆"
                        : ""
                }
            </td>

        `;

        table.appendChild(row);
    });
    
    showPerformanceChart(results);
}

const themeButton = document.getElementById("themeToggle");

themeButton.addEventListener(
    "click",
    () => {

        document.body.classList.toggle(
            "dark"
        );


        if(
            document.body.classList.contains("dark")
        ){

            themeButton.innerHTML =
            "☀ Light Mode";

        }
        else{

            themeButton.innerHTML =
            "🌙 Dark Mode";

        }

    }
);

// ---------------------------
// Random Workload
// ---------------------------

document
    .getElementById("random")
    .addEventListener("click", () => {

        processes = [];

        const count = 8;

        for (let i = 0; i < count; i++) {

            processes.push({

                pid: i + 1,

                arrivalTime:
                    Math.floor(
                        Math.random() * 10
                    ),

                burstTime:
                    Math.floor(
                        Math.random() * 10
                    ) + 1,

                priority:
                    Math.floor(
                        Math.random() * 5
                    ) + 1
            });
        }

        renderProcesses();
    });


// ---------------------------
// Clear
// ---------------------------

document
    .getElementById("clear")
    .addEventListener("click", () => {

        processes = [];

        renderProcesses();

        document
            .getElementById("results")
            .classList.add("hidden");

        document
            .getElementById("comparison")
            .classList.add("hidden");
    });