#pragma once

#include "DataRecord.h"
#include <vector>
#include <memory>
#include <shared_mutex>

namespace BondForge {
namespace Core {
namespace Data {

/**
 * @brief 数据服务接口
 * 
 * 提供数据的增删改查等基础操作
 */
class IDataService {
public:
    virtual ~IDataService() = default;
    
    /**
     * @brief 添加数据记录
     * 
     * @param record 要添加的数据记录
     * @return 是否成功
     */
    virtual bool addData(const DataRecord& record) = 0;
    
    /**
     * @brief 删除数据记录
     * 
     * @param id 要删除的数据记录ID
     * @return 是否成功
     */
    virtual bool deleteData(const std::string& id) = 0;
    
    /**
     * @brief 更新数据记录
     * 
     * @param record 要更新的数据记录
     * @return 是否成功
     */
    virtual bool updateData(const DataRecord& record) = 0;
    
    /**
     * @brief 获取单个数据记录
     * 
     * @param id 数据记录ID
     * @return 数据记录（如果存在）
     */
    virtual std::unique_ptr<DataRecord> getData(const std::string& id) = 0;
    
    /**
     * @brief 获取所有数据记录
     * 
     * @return 所有数据记录的列表
     */
    virtual std::vector<DataRecord> getAllData() = 0;
    
    /**
     * @brief 根据条件查询数据记录
     * 
     * @param category 分类过滤（空字符串表示不过滤）
     * @param tags 标签过滤（空集合表示不过滤）
     * @return 符合条件的数据记录列表
     */
    virtual std::vector<DataRecord> queryData(
        const std::string& category = "",
        const std::unordered_set<std::string>& tags = {}) = 0;
};

/**
 * @brief 数据服务实现类
 * 
 * 使用内存存储数据记录（实际生产中可替换为数据库实现）
 */
class DataService : public IDataService {
private:
    std::vector<DataRecord> m_records;
    mutable std::shared_mutex m_mutex;
    
public:
    bool addData(const DataRecord& record) override;
    bool deleteData(const std::string& id) override;
    bool updateData(const DataRecord& record) override;
    std::unique_ptr<DataRecord> getData(const std::string& id) override;
    std::vector<DataRecord> getAllData() override;
    std::vector<DataRecord> queryData(
        const std::string& category = "",
        const std::unordered_set<std::string>& tags = {}) override;
};

} // namespace Data
} // namespace Core
} // namespace BondForge