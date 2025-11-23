# BondForge V2.0 示例文件 / Sample Files

本目录包含了BondForge V2.0的示例文件，帮助您快速了解和使用软件的各项功能。

This directory contains sample files for BondForge V2.0, helping you quickly understand and use the software's features.

## 目录结构 / Directory Structure

```
examples/
├── sample_examples.json    # 整合的示例数据文件 / Integrated sample data file
├── molecules/              # 分子结构文件示例 / Molecular structure file examples
│   ├── benzene.sdf         # 苯分子 / Benzene molecule
│   ├── aspirin.sdf         # 阿司匹林 / Aspirin
│   └── caffeine.mol        # 咖啡因 / Caffeine
├── data/                   # 数据文件示例 / Data file examples
│   ├── molecular_properties.csv  # 分子属性数据 / Molecular properties data
│   ├── solubility_data.csv       # 溶解度数据 / Solubility data
│   └── protein_sequences.fasta   # 蛋白质序列 / Protein sequences
├── models/                 # 模型文件示例 / Model file examples
│   └── linear_regression_example.json  # 线性回归示例 / Linear regression example
└── projects/               # 项目文件示例 / Project file examples
    └── drug_discovery_project.json   # 药物发现项目示例 / Drug discovery project example
```

## 使用方法 / Usage

### 分子可视化 / Molecular Visualization

导入分子文件进行可视化：

```python
# 示例代码
from bondforge import MoleculeViewer

viewer = MoleculeViewer()
viewer.load("examples/molecules/benzene.sdf")
viewer.show()
```

### 数据分析 / Data Analysis

使用示例数据集进行机器学习分析：

```python
# 示例代码
from bondforge import DataLoader, MLTrainer

# 加载数据
loader = DataLoader()
data = loader.load_csv("examples/data/molecular_properties.csv")

# 训练模型
trainer = MLTrainer()
model = trainer.train_regression(data, target="molecular_weight")
```

### 项目加载 / Project Loading

加载示例项目：

```python
# 示例代码
from bondforge import Project

project = Project.load("examples/projects/drug_discovery_project.json")
project.execute()
```

## 示例说明 / Example Descriptions

### 分子文件 / Molecule Files

- **benzene.sdf**: 苯分子的SDF格式文件，包含原子坐标和键信息
- **aspirin.sdf**: 阿司匹林的SDF格式文件，展示了药物分子的结构
- **caffeine.mol**: 咖啡因的MOL格式文件，展示了生物活性分子的结构

### 数据文件 / Data Files

- **molecular_properties.csv**: 包含分子的各种物化性质，可用于回归分析
- **solubility_data.csv**: 包含实验测定的溶解度数据，可用于溶解度预测模型
- **protein_sequences.fasta**: 包含蛋白质序列数据，可用于生物信息学分析

### 模型文件 / Model Files

- **linear_regression_example.json**: 线性回归模型示例，展示了如何保存和加载机器学习模型

### 项目文件 / Project Files

- **drug_discovery_project.json**: 药物发现项目示例，展示了完整的工作流程和项目组织方式

## 更多信息 / More Information

有关更多详细信息，请参阅：
For more detailed information, please refer to:
- [综合指南](../COMPREHENSIVE_GUIDE.md) / [Comprehensive Guide](../COMPREHENSIVE_GUIDE.md)
- [开发者指南](../docs/developer_guide.md) / [Developer Guide](../docs/developer_guide.md)