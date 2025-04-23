# COP5725BClean


# BClean (C++ Reimplementation)

This repository contains a C++ reimplementation of **BClean**, a data cleaning system that detects and repairs data errors using probabilistic inference and user-defined constraints.

It replicates the key components of the original [BClean (NeurIPS 2023)](https://arxiv.org/abs/2311.06517) system, including:

- Error cell detection
- Conditional dependency learning via Bayesian Networks
- User Constraint (UC)-aware inference
- Evaluation metrics (Precision, Recall, F1)

---

## Project Structure

BClean/
├── BayesianClean.*         # main cleaning and inference engine
├── dataset.*               # data loading and error detection
├── include/                # common headers 
├── src/                    # modules: BNStructure, Compensative, Inference.
├── examples/
│   ├── beers.cpp           # demo entry point
│   ├── Makefile            # build script
├── data/
│   ├── dirty.csv           # corrupted dataset
│   └── clean.csv           # ground truth
├── json/
│   └── beers.json          # UC file (user constraints)

---

## Build Instructions

This project uses a simple Makefile to compile everything. From the `examples/` directory:

```bash
cd examples
make

This will compile all source files and produce an executable named beers.

⸻

Running the Project

Once built, you can run the program directly:

./beers

Optional flags allow testing different BClean variants:

./beers -UC     # Disable user constraints (baseline version)
./beers -PI     # Enable Partition Inference only
./beers -PIP    # Partition Inference + Pruning

No arguments will run the default UC-enabled version.

⸻

Output

The program prints out evaluation metrics based on simulated repair accuracy:

===== Evaluating Repair Results =====
+++all repair: 1034
lack of 
miss_err: 427, pre_right: 623
Repair Pre: 0.712, Recall: 0.593, F1-score: 0.647
++++++++++++++++++++time using: 12.35+++++++++++++++++++++++
date: Sun Apr 20 15:41:12 2025

These scores reflect the system’s ability to correctly detect and repair dirty cells.

⸻

Dataset

The data/ directory contains:
	•	dirty.csv: a corrupted dataset
	•	clean.csv: the clean ground truth
	•	json/beers.json: a JSON file defining user constraints used for inference

You can replace these with your own datasets, following the same structure.

⸻

Reference

This project is based on the paper:

BClean: Bayesian Data Repair by Conditional Dependency Learning
Xiangyu Ke, Lin Gu, Kexuan Sun, Jingbo Shang
NeurIPS 2023
arXiv:2311.06517
