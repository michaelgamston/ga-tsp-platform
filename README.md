# Distributed TSP Solver

![CI](https://github.com/michaelgamston/ga-tsp-platform/actions/workflows/ci.yml/badge.svg)

A cloud-native system that solves the **Travelling Salesman Problem (TSP)** using a
**genetic algorithm**, designed to demonstrate **Docker, Kubernetes and Azure** in a
realistic, scalable architecture.

> 🚧 **Actively in development.** See [Progress](#progress) and [Next steps](#next-steps).

---

## The project

Users submit a map (a matrix of distances between locations) and the system searches
for the shortest route that visits every location once. Each request becomes a job on a
queue, processed by worker containers that **scale automatically with demand**: more
jobs start more workers, and the workers scale back to zero when the queue is empty.

### Planned architecture
```
Web page → API → Job queue (Redis) → Worker containers → C++ GA engine
                       ↘  Postgres (maps, jobs, results)  ↙
```

| Component | Technology | Role |
|---|---|---|
| Solver engine | C++17, CMake, GoogleTest | Evolves candidate routes using a genetic algorithm |
| API & workers | *(evaluating which language to use)* | Accept jobs, queue them, run the engine, report progress |
| Infrastructure | Docker, Kubernetes (AKS), Helm, KEDA | Containerised services with queue-driven autoscaling |
| Cloud & delivery | Azure, Terraform, GitHub Actions | Infrastructure as code; automated build, test and deploy |

---

## What I'm aiming to learn

- **Containerisation:** packaging multiple services with Docker; multi-stage builds
- **Orchestration:** deploying and autoscaling workloads on Kubernetes
- **Cloud infrastructure:** provisioning Azure resources reproducibly with Terraform
- **CI/CD:** automated pipelines that test and deploy every change
- **Modern C++:** project structure, build systems, input validation, unit testing
- **Professional workflow:** feature branches, pull requests, protected `main`, test-driven development

---

## Progress

### ✅ Done
- **Distance-matrix parser** with thorough input validation:
  - rejects non-numeric values, decimals, values that are too large, empty cells, empty input,
    non-square matrices and invalid negative values
  - enforces a map size of 3–50 nodes, which keeps tour lengths well within integer limits
  - accepts trailing commas, surrounding whitespace and Windows (`\r\n`) line endings
  - error messages that report the offending value and line number
- **`Individual` (a candidate route)**, encapsulated in its own class:
  - genome is a random permutation of the map's nodes, reproducible from a seed
  - fitness is the total tour length (including the return to the start), with a penalty for
    every unconnected edge, and is recalculated automatically whenever the genome changes
  - private state behind getters and a validating setter, which rejects duplicate nodes,
    out-of-range nodes and genomes of the wrong length, and leaves the route unchanged when it does
- **Project structure:** `include/` / `src/` / `tests/` layout, with the engine code built as a
  reusable library (`tsp_core`)
- **CMake build** with compiler warnings enabled
- **44 unit tests** (GoogleTest) across the parser and `Individual`, covering pass, fail, boundary and
  real-data cases, written largely test-first. These tests caught real bugs, including two crashes
  from out-of-range memory access, before the code was merged
- **Continuous integration:** GitHub Actions builds and tests every pull request
- **Workflow:** all changes go through feature branches and reviewed pull requests

### 🔨 In progress
- **Genetic algorithm engine:** moving `Genetic_algorithm` into its own files, then adding
  selection, crossover and mutation

---

## Next steps

1. **Finish the GA engine:** tournament selection, ordered crossover (OX) and inversion mutation,
   each with unit tests, plus the generation loop, validated against the known optimal tours
   of the example maps
2. **Engine output:** stream progress (best and average tour length per generation) as JSON lines
3. **API and worker services:** job submission, queueing (Redis) and result storage (Postgres)
4. **Docker:** containerise each service and run everything locally with docker-compose
5. **Kubernetes:** deploy locally, then package with Helm
6. **Azure:** provision AKS and a container registry with Terraform, then deploy through CI/CD
7. **Autoscaling demo:** KEDA scales workers with queue length (0 → N → 0)

---

## Running it locally

**Requirements:** CMake ≥ 3.22 and a C++17 compiler (GCC 11+)

```bash
cd tsp_engine
cmake -S . -B build
cmake --build build

# run the tests
ctest --test-dir build --output-on-failure

# run the engine on an example map
./build/tsp_engine < ../maps/map_05.csv
```

Example maps and the input format are described in [maps/README.md](maps/README.md).

---

## Repository layout
```
.github/workflows/   CI pipeline
maps/                Example distance-matrix maps (with known optimal tour lengths)
tsp_engine/
├── include/         Public headers
├── src/             Engine source code
└── tests/           GoogleTest unit tests
```
