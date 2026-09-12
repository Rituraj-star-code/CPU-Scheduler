const express = require("express");
const path = require("path");
const { spawn } = require("child_process");

const app = express();

const PORT = 3000;

app.use(express.json());

app.use(
    express.static(
        path.join(__dirname, "..", "frontend")
    )
);

function runScheduler(data) {

    return new Promise((resolve, reject) => {

        const exePath = path.join(
            __dirname,
            "scheduler.exe"
        );

        const child = spawn(exePath);

        let output = "";
        let errorOutput = "";

        child.stdout.on("data", (data) => {

            output += data.toString();
        });

        child.stderr.on("data", (data) => {

            errorOutput += data.toString();
        });

        child.on("error", (error) => {

            reject(error);
        });

        child.on("close", (code) => {

            if (code !== 0) {

                reject(
                    new Error(
                        errorOutput ||
                        "C++ scheduler failed"
                    )
                );

                return;
            }

            try {

                const result =
                    JSON.parse(output);

                resolve(result);

            } catch (error) {

                reject(
                    new Error(
                        "Invalid JSON returned by C++"
                    )
                );
            }
        });

        // Send input to C++
        child.stdin.write(
            data.algorithm + "\n"
        );

        child.stdin.write(
            data.quantum + "\n"
        );

        child.stdin.write(
            data.contextSwitch + "\n"
        );

        child.stdin.write(
            data.starvationThreshold + "\n"
        );

        child.stdin.write(
            data.processes.length + "\n"
        );

        for (const p of data.processes) {

            child.stdin.write(
                `${p.pid} ${p.arrivalTime} ${p.burstTime} ${p.priority}\n`
            );
        }

        child.stdin.end();
    });
}

app.post("/api/schedule", async (req, res) => {

    try {

        const result =
            await runScheduler(req.body);

        res.json(result);

    } catch (error) {

        console.error(error);

        res.status(500).json({
            error: error.message
        });
    }
});

app.post("/api/compare", async (req, res) => {

    try {

        const algorithms = [
            "FCFS",
            "SJF",
            "SRTF",
            "PRIORITY",
            "PREEMPTIVE_PRIORITY",
            "RR",
            "MLQ",
            "MLFQ"
        ];

        const results = [];

        for (const algorithm of algorithms) {

            console.log("Running algorithm:", algorithm);

            const data = {
                ...req.body,
                algorithm
            };

            const result = await runScheduler(data);

            results.push(result);
        }

        res.json(results);

    } catch (error) {

        console.error(error);

        res.status(500).json({
            error: error.message
        });
    }
});

app.get("/", (req, res) => {

    res.sendFile(
        path.join(
            __dirname,
            "..",
            "frontend",
            "index.html"
        )
    );
});

app.listen(PORT, () => {

    console.log(
        `Server running at http://localhost:${PORT}`
    );
});