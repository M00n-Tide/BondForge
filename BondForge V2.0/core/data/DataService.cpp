#include "DataService.h"
#include <algorithm>

namespace BondForge {
namespace Core {
namespace Data {

bool DataService::addData(const DataRecord& record) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    // 检查ID是否已存在
    auto it = std::find_if(m_records.begin(), m_records.end(),
        [&record](const DataRecord& r) { return r.id == record.id; });
    
    if (it != m_records.end()) {
        return false; // ID已存在
    }
    
    m_records.push_back(record);
    return true;
}

bool DataService::deleteData(const std::string& id) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    auto it = std::find_if(m_records.begin(), m_records.end(),
        [&id](const DataRecord& r) { return r.id == id; });
    
    if (it != m_records.end()) {
        m_records.erase(it);
        return true;
    }
    
    return false; // 未找到
}

bool DataService::updateData(const DataRecord& record) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    auto it = std::find_if(m_records.begin(), m_records.end(),
        [&record](const DataRecord& r) { return r.id == record.id; });
    
    if (it != m_records.end()) {
        *it = record;
        return true;
    }
    
    return false; // 未找到
}

std::unique_ptr<DataRecord> DataService::getData(const std::string& id) {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    auto it = std::find_if(m_records.begin(), m_records.end(),
        [&id](const DataRecord& r) { return r.id == id; });
    
    if (it != m_records.end()) {
        return std::make_unique<DataRecord>(*it);
    }
    
    return nullptr; // 未找到
}

std::vector<DataRecord> DataService::getAllData() {
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_records; // 返回副本
}

std::vector<DataRecord> DataService::queryData(
    const std::string& category,
    const std::unordered_set<std::string>& tags) {
    
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    std::vector<DataRecord> result;
    
    for (const auto& record : m_records) {
        bool match = true;
        
        // 检查分类
        if (!category.empty() && record.category != category) {
            match = false;
        }
        
        // 检查标签（至少包含一个指定的标签）
        if (!tags.empty()) {
            bool hasAnyTag = false;
            for (const auto& tag : tags) {
                if (record.tags.count(tag) > 0) {
                    hasAnyTag = true;
                    break;
                }
            }
            if (!hasAnyTag) {
                match = false;
            }
        }
        
        if (match) {
            result.push_back(record);
        }
    }
    
    return result;
}

} // namespace Data
} // namespace Core
} // namespace BondForge