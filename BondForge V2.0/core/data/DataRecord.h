#pragma once

#include <string>
#include <unordered_set>
#include <cstdint>

namespace BondForge {
namespace Core {
namespace Data {

/**
 * @brief 数据记录结构体
 * 
 * 表示化学数据记录的基本信息
 */
struct DataRecord {
    std::string id;                // 数据唯一标识
    std::string content;           // 数据内容（如化学式、分子结构）
    std::string format;            // 数据格式（CSV/JSON/SDF等）
    std::unordered_set<std::string> tags;  // 数据标签集合
    std::string category;          // 数据分类
    std::string uploader;          // 上传用户
    uint64_t timestamp;            // 上传时间戳
    
    /**
     * @brief 序列化标签为字符串
     * 
     * @return 序列化后的标签字符串
     */
    std::string serializeTags() const {
        std::string result;
        for (const auto& tag : tags) {
            if (!result.empty()) result += ",";
            result += tag;
        }
        return result;
    }
    
    /**
     * @brief 从字符串反序列化标签
     * 
     * @param tagString 逗号分隔的标签字符串
     */
    void deserializeTags(const std::string& tagString) {
        tags.clear();
        std::stringstream ss(tagString);
        std::string tag;
        while (std::getline(ss, tag, ',')) {
            if (!tag.empty()) {
                tags.insert(tag);
            }
        }
    }
};

} // namespace Data
} // namespace Core
} // namespace BondForge