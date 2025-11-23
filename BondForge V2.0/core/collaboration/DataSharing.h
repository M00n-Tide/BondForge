#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "../data/DataRecord.h"
#include "User.h"

namespace BondForge {
namespace Core {
namespace Collaboration {

/**
 * @brief 共享权限枚举
 */
enum class SharePermission {
    ReadOnly,       // 只读
    ReadWrite,      // 读写
    Manage          // 管理（包括删除和分享）
};

/**
 * @brief 共享有效期枚举
 */
enum class ShareExpiry {
    Permanent,      // 永久
    OneDay,         // 一天
    SevenDays,      // 七天
    ThirtyDays,      // 三十天
    NinetyDays       // 九十天
};

/**
 * @brief 数据共享记录
 */
struct ShareRecord {
    std::string id;                                   // 共享ID
    std::string dataId;                               // 被共享的数据ID
    std::string ownerId;                              // 数据所有者ID
    std::string sharedWithUserId;                        // 共享给的用户ID（如果是公开共享则为空）
    std::string shareToken;                            // 共享令牌
    SharePermission permission;                          // 共享权限
    ShareExpiry expiry;                               // 有效期
    std::chrono::system_clock::time_point createdAt;        // 创建时间
    std::chrono::system_clock::time_point expiresAt;      // 过期时间
    bool isActive;                                     // 是否激活
    std::string description;                           // 描述
    int downloadCount;                                 // 下载次数
    std::chrono::system_clock::time_point lastAccessedAt;  // 最后访问时间
};

/**
 * @brief 数据共享服务接口
 */
class IDataSharingService {
public:
    virtual ~IDataSharingService() = default;
    
    /**
     * @brief 创建数据共享
     * 
     * @param dataId 要共享的数据ID
     * @param ownerId 数据所有者ID
     * @param sharedWithUserId 共享给的用户ID（可选，空表示公开共享）
     * @param permission 共享权限
     * @param expiry 有效期
     * @param description 描述
     * @return 共享记录ID（成功）或空字符串（失败）
     */
    virtual std::string createShare(
        const std::string& dataId,
        const std::string& ownerId,
        const std::string& sharedWithUserId = "",
        SharePermission permission = SharePermission::ReadOnly,
        ShareExpiry expiry = ShareExpiry::Permanent,
        const std::string& description = "") = 0;
    
    /**
     * @brief 获取共享记录
     * 
     * @param shareId 共享ID
     * @return 共享记录（如果存在）
     */
    virtual std::unique_ptr<ShareRecord> getShare(const std::string& shareId) = 0;
    
    /**
     * @brief 根据共享令牌获取共享记录
     * 
     * @param token 共享令牌
     * @return 共享记录（如果存在）
     */
    virtual std::unique_ptr<ShareRecord> getShareByToken(const std::string& token) = 0;
    
    /**
     * @brief 获取用户拥有的共享（共享出去的）
     * 
     * @param userId 用户ID
     * @return 共享记录列表
     */
    virtual std::vector<ShareRecord> getUserShares(const std::string& userId) = 0;
    
    /**
     * @brief 获取共享给用户的（用户被共享的）
     * 
     * @param userId 用户ID
     * @return 共享记录列表
     */
    virtual std::vector<ShareRecord> getSharesWithUser(const std::string& userId) = 0;
    
    /**
     * @brief 获取所有活跃的公开共享
     * 
     * @return 共享记录列表
     */
    virtual std::vector<ShareRecord> getPublicShares() = 0;
    
    /**
     * @brief 更新共享记录
     * 
     * @param share 共享记录
     * @return 是否成功
     */
    virtual bool updateShare(const ShareRecord& share) = 0;
    
    /**
     * @brief 删除共享
     * 
     * @param shareId 共享ID
     * @param requestingUserId 请求用户的ID（用于权限检查）
     * @return 是否成功
     */
    virtual bool deleteShare(const std::string& shareId, const std::string& requestingUserId) = 0;
    
    /**
     * @brief 检查用户是否可以通过共享访问数据
     * 
     * @param shareId 共享ID
     * @param userId 用户ID
     * @return 是否可以访问
     */
    virtual bool canAccessData(const std::string& shareId, const std::string& userId) = 0;
    
    /**
     * @brief 生成随机共享令牌
     * 
     * @return 随机令牌
     */
    virtual std::string generateShareToken() = 0;
    
    /**
     * @brief 清理过期的共享
     * 
     * @return 清理的共享数量
     */
    virtual int cleanupExpiredShares() = 0;
};

/**
 * @brief 数据共享服务实现类
 * 
 * 使用内存存储共享记录（实际生产中可替换为数据库实现）
 */
class DataSharingService : public IDataSharingService {
private:
    std::vector<ShareRecord> m_shares;
    std::unique_ptr<IDataService> m_dataService;
    std::unique_ptr<IUserService> m_userService;
    
    /**
     * @brief 计算过期时间
     * 
     * @param createdAt 创建时间
     * @param expiry 有效期类型
     * @return 过期时间
     */
    std::chrono::system_clock::time_point calculateExpiryTime(
        const std::chrono::system_clock::time_point& createdAt,
        ShareExpiry expiry);
    
    /**
     * @brief 检查共享是否已过期
     * 
     * @param share 共享记录
     * @return 是否过期
     */
    bool isShareExpired(const ShareRecord& share);
    
    /**
     * @brief 更新共享的访问统计
     * 
     * @param shareId 共享ID
     */
    void updateShareAccessStats(const std::string& shareId);
    
public:
    DataSharingService(
        std::unique_ptr<IDataService> dataService,
        std::unique_ptr<IUserService> userService);
    
    std::string createShare(
        const std::string& dataId,
        const std::string& ownerId,
        const std::string& sharedWithUserId = "",
        SharePermission permission = SharePermission::ReadOnly,
        ShareExpiry expiry = ShareExpiry::Permanent,
        const std::string& description = "") override;
    
    std::unique_ptr<ShareRecord> getShare(const std::string& shareId) override;
    std::unique_ptr<ShareRecord> getShareByToken(const std::string& token) override;
    std::vector<ShareRecord> getUserShares(const std::string& userId) override;
    std::vector<ShareRecord> getSharesWithUser(const std::string& userId) override;
    std::vector<ShareRecord> getPublicShares() override;
    bool updateShare(const ShareRecord& share) override;
    bool deleteShare(const std::string& shareId, const std::string& requestingUserId) override;
    bool canAccessData(const std::string& shareId, const std::string& userId) override;
    std::string generateShareToken() override;
    int cleanupExpiredShares() override;
};

/**
 * @brief 共享权限工具
 */
class SharingUtils {
public:
    /**
     * @brief 将权限转换为字符串
     * 
     * @param permission 权限
     * @return 字符串表示
     */
    static std::string permissionToString(SharePermission permission);
    
    /**
     * @brief 将有效期转换为字符串
     * 
     * @param expiry 有效期
     * @return 字符串表示
     */
    static std::string expiryToString(ShareExpiry expiry);
    
    /**
     * @brief 计算剩余时间（以人类可读格式）
     * 
     * @param expiresAt 过期时间
     * @return 格式化的剩余时间字符串
     */
    static std::string formatTimeRemaining(
        const std::chrono::system_clock::time_point& expiresAt);
    
    /**
     * @brief 检查共享权限是否允许操作
     * 
     * @param permission 共享权限
     * @param operation 操作
     * @return 是否允许
     */
    static bool permissionAllowsOperation(
        SharePermission permission,
        const std::string& operation); // "read", "write", "delete"
};

} // namespace Collaboration
} // namespace Core
} // namespace BondForge