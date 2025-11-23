#pragma once

#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "../data/DataRecord.h"

#ifdef USE_MLPACK
#include <mlpack/core.hpp>
#include <mlpack/methods/linear_regression/linear_regression.hpp>
#include <mlpack/methods/logistic_regression/logistic_regression.hpp>
#include <mlpack/methods/kmeans/kmeans.hpp>
#include <mlpack/methods/decision_tree/decision_tree.hpp>
#endif

namespace BondForge {
namespace Core {
namespace ML {

/**
 * @brief 机器学习模型类型枚举
 */
enum class ModelType {
    LinearRegression,
    LogisticRegression,
    DecisionTree,
    KMeans,
    TimeSeries
};

/**
 * @brief 模型训练结果
 */
struct TrainingResult {
    bool success;
    double accuracy;
    double precision;
    double recall;
    double f1Score;
    double meanSquaredError;
    std::map<std::string, double> additionalMetrics;
    std::string errorMessage;
};

/**
 * @brief 机器学习模型接口
 */
class IMLModel {
public:
    virtual ~IMLModel() = default;
    
    /**
     * @brief 训练模型
     * 
     * @param trainingData 训练数据
     * @param trainingLabels 训练标签
     * @param parameters 训练参数
     * @return 训练结果
     */
    virtual TrainingResult train(
        const std::vector<std::vector<double>>& trainingData,
        const std::vector<double>& trainingLabels,
        const std::map<std::string, double>& parameters = {}) = 0;
    
    /**
     * @brief 预测
     * 
     * @param testData 测试数据
     * @return 预测结果
     */
    virtual std::vector<double> predict(
        const std::vector<std::vector<double>>& testData) = 0;
    
    /**
     * @brief 获取模型类型
     * 
     * @return 模型类型
     */
    virtual ModelType getModelType() const = 0;
    
    /**
     * @brief 保存模型到文件
     * 
     * @param filePath 文件路径
     * @return 是否成功
     */
    virtual bool saveModel(const std::string& filePath) = 0;
    
    /**
     * @brief 从文件加载模型
     * 
     * @param filePath 文件路径
     * @return 是否成功
     */
    virtual bool loadModel(const std::string& filePath) = 0;
};

/**
 * @brief 数据预处理工具
 */
class DataPreprocessor {
public:
    /**
     * @brief 特征提取 - 从化学数据记录中提取数值特征
     * 
     * @param records 数据记录列表
     * @param featureType 特征类型
     * @return 特征矩阵
     */
    static std::vector<std::vector<double>> extractFeatures(
        const std::vector<Data::DataRecord>& records,
        const std::string& featureType = "content_length");
    
    /**
     * @brief 标签提取 - 从化学数据记录中提取标签
     * 
     * @param records 数据记录列表
     * @param labelType 标签类型
     * @return 标签向量
     */
    static std::vector<double> extractLabels(
        const std::vector<Data::DataRecord>& records,
        const std::string& labelType = "category");
    
    /**
     * @brief 数据标准化
     * 
     * @param data 输入数据
     * @return 标准化后的数据
     */
    static std::vector<std::vector<double>> normalize(
        const std::vector<std::vector<double>>& data);
    
    /**
     * @brief 数据分割 - 将数据分为训练集和测试集
     * 
     * @param data 输入数据
     * @param labels 输入标签
     * @param trainRatio 训练集比例
     * @return 分割后的数据
     */
    static std::tuple<
        std::vector<std::vector<double>>, std::vector<std::vector<double>>,
        std::vector<double>, std::vector<double>
    > splitTrainTest(
        const std::vector<std::vector<double>>& data,
        const std::vector<double>& labels,
        double trainRatio = 0.8);
};

#ifdef USE_MLPACK
/**
 * @brief 基于mlpack的线性回归模型
 */
class MlpackLinearRegression : public IMLModel {
private:
    std::unique_ptr<mlpack::regression::LinearRegression> m_model;
    arma::mat m_features;
    arma::colvec m_labels;
    
public:
    TrainingResult train(
        const std::vector<std::vector<double>>& trainingData,
        const std::vector<double>& trainingLabels,
        const std::map<std::string, double>& parameters = {}) override;
    
    std::vector<double> predict(
        const std::vector<std::vector<double>>& testData) override;
    
    ModelType getModelType() const override { return ModelType::LinearRegression; }
    
    bool saveModel(const std::string& filePath) override;
    bool loadModel(const std::string& filePath) override;
};

/**
 * @brief 基于mlpack的逻辑回归模型
 */
class MlpackLogisticRegression : public IMLModel {
private:
    std::unique_ptr<mlpack::regression::LogisticRegression<>> m_model;
    arma::mat m_features;
    arma::Row<size_t> m_labels;
    
public:
    TrainingResult train(
        const std::vector<std::vector<double>>& trainingData,
        const std::vector<double>& trainingLabels,
        const std::map<std::string, double>& parameters = {}) override;
    
    std::vector<double> predict(
        const std::vector<std::vector<double>>& testData) override;
    
    ModelType getModelType() const override { return ModelType::LogisticRegression; }
    
    bool saveModel(const std::string& filePath) override;
    bool loadModel(const std::string& filePath) override;
};

/**
 * @brief 基于mlpack的决策树模型
 */
class MlpackDecisionTree : public IMLModel {
private:
    std::unique_ptr<mlpack::tree::DecisionTree<>> m_model;
    arma::mat m_features;
    arma::Row<size_t> m_labels;
    
public:
    TrainingResult train(
        const std::vector<std::vector<double>>& trainingData,
        const std::vector<double>& trainingLabels,
        const std::map<std::string, double>& parameters = {}) override;
    
    std::vector<double> predict(
        const std::vector<std::vector<double>>& testData) override;
    
    ModelType getModelType() const override { return ModelType::DecisionTree; }
    
    bool saveModel(const std::string& filePath) override;
    bool loadModel(const std::string& filePath) override;
};

/**
 * @brief 基于mlpack的K均值聚类模型
 */
class MlpackKMeans : public IMLModel {
private:
    std::unique_ptr<mlpack::kmeans::KMeans<>> m_model;
    arma::mat m_features;
    arma::Row<size_t> m_assignments;
    arma::mat m_centroids;
    
public:
    TrainingResult train(
        const std::vector<std::vector<double>>& trainingData,
        const std::vector<double>& trainingLabels,
        const std::map<std::string, double>& parameters = {}) override;
    
    std::vector<double> predict(
        const std::vector<std::vector<double>>& testData) override;
    
    ModelType getModelType() const override { return ModelType::KMeans; }
    
    bool saveModel(const std::string& filePath) override;
    bool loadModel(const std::string& filePath) override;
};
#endif

/**
 * @brief 模拟机器学习模型（当没有mlpack时使用）
 */
class MockMLModel : public IMLModel {
private:
    ModelType m_modelType;
    std::vector<std::vector<double>> m_trainingData;
    std::vector<double> m_trainingLabels;
    
public:
    explicit MockMLModel(ModelType type) : m_modelType(type) {}
    
    TrainingResult train(
        const std::vector<std::vector<double>>& trainingData,
        const std::vector<double>& trainingLabels,
        const std::map<std::string, double>& parameters = {}) override;
    
    std::vector<double> predict(
        const std::vector<std::vector<double>>& testData) override;
    
    ModelType getModelType() const override { return m_modelType; }
    
    bool saveModel(const std::string& filePath) override;
    bool loadModel(const std::string& filePath) override;
};

/**
 * @brief 模型工厂
 */
class ModelFactory {
public:
    /**
     * @brief 创建指定类型的模型
     * 
     * @param type 模型类型
     * @return 模型实例
     */
    static std::unique_ptr<IMLModel> createModel(ModelType type);
    
    /**
     * @brief 获取所有可用的模型类型
     * 
     * @return 模型类型列表
     */
    static std::vector<ModelType> getAvailableModels();
    
    /**
     * @brief 将模型类型转换为字符串
     * 
     * @param type 模型类型
     * @return 字符串表示
     */
    static std::string modelTypeToString(ModelType type);
    
    /**
     * @brief 从字符串转换为模型类型
     * 
     * @param str 字符串表示
     * @return 模型类型
     */
    static ModelType stringToModelType(const std::string& str);
};

} // namespace ML
} // namespace Core
} // namespace BondForge