#include "DataSharing.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace BondForge {
namespace Core {
namespace Collaboration {

// DataSharingService 实现
DataSharingService::DataSharingService(
    std::unique_ptr<IDataService> dataService,
    std::unique_ptr<IUserService> userService)
    : m_dataService(std::move(dataService))
    , m_userService(std::move(userService)) {
    
    // 定期清理过期共享（在实际应用中可以使用后台线程）
    cleanupExpiredShares();
}

std::string DataSharingService::createShare(
    const std::string& dataId,
    const std::string& ownerId,
    const std::string& sharedWithUserId,
    SharePermission permission,
    ShareExpiry expiry,
    const std::string& description) {
    
    // 验证数据是否存在
    auto dataRecord = m_dataService->getData(dataId);
    if (!dataRecord) {
        return ""; // 数据不存在
    }
    
    // 验证所有者是否有权限分享数据
    if (!m_userService->hasPermission(ownerId, Permission::Share)) {
        return ""; // 无权限分享
    }
    
    // 验证目标用户是否存在（如果指定）
    if (!sharedWithUserId.empty()) {
        auto targetUser = m_userService->getUser(sharedWithUserId);
        if (!targetUser) {
            return ""; // 目标用户不存在
        }
    }
    
    // 创建共享记录
    ShareRecord share;
    share.id = "share-" + std::to_string(m_shares.size() + 1);
    share.dataId = dataId;
    share.ownerId = ownerId;
    share.sharedWithUserId = sharedWithUserId;
    share.shareToken = generateShareToken();
    share.permission = permission;
    share.expiry = expiry;
    share.createdAt = std::chrono::system_clock::now();
    share.expiresAt = calculateExpiryTime(share.createdAt, expiry);
    share.isActive = true;
    share.description = description;
    share.downloadCount = 0;
    share.lastAccessedAt = std::chrono::system_clock::time_point{};
    
    // 添加到共享列表
    m_shares.push_back(share);
    
    return share.id;
}

std::unique_ptr<ShareRecord> DataSharingService::getShare(const std::string& shareId) {
    for (const auto& share : m_shares) {
        if (share.id == shareId) {
            return std::make_unique<ShareRecord>(share);
        }
    }
    
    return nullptr; // 共享不存在
}

std::unique_ptr<ShareRecord> DataSharingService::getShareByToken(const std::string& token) {
    for (const auto& share : m_shares) {
        if (share.shareToken == token) {
            // 检查是否过期
            if (isShareExpired(share)) {
                return nullptr;
            }
            
            // 更新访问统计
            updateShareAccessStats(share.id);
            
            return std::make_unique<ShareRecord>(share);
        }
    }
    
    return nullptr; // 共享不存在或已过期
}

std::vector<ShareRecord> DataSharingService::getUserShares(const std::string& userId) {
    std::vector<ShareRecord> userShares;
    
    for (const auto& share : m_shares) {
        if (share.ownerId == userId) {
            userShares.push_back(share);
        }
    }
    
    return userShares;
}

std::vector<ShareRecord> DataSharingService::getSharesWithUser(const std::string& userId) {
    std::vector<ShareRecord> sharedWithUser;
    
    for (const auto& share : m_shares) {
        if (share.sharedWithUserId == userId) {
            // 不返回过期的共享
            if (!isShareExpired(share)) {
                sharedWithUser.push_back(share);
            }
        }
    }
    
    return sharedWithUser;
}

std::vector<ShareRecord> DataSharingService::getPublicShares() {
    std::vector<ShareRecord> publicShares;
    
    for (const auto& share : m_shares) {
        // 公共共享是指sharedWithUserId为空的共享
        if (share.sharedWithUserId.empty() && !isShareExpired(share)) {
            publicShares.push_back(share);
        }
    }
    
    return publicShares;
}

bool DataSharingService::updateShare(const ShareRecord& share) {
    for (auto& existingShare : m_shares) {
        if (existingShare.id == share.id) {
            // 验证请求用户是否有权限修改此共享
            if (!m_userService->hasPermission(existingShare.ownerId, Permission::Share)) {
                return false;
            }
            
            existingShare = share;
            return true;
        }
    }
    
    return false; // 共享不存在
}

bool DataSharingService::deleteShare(const std::string& shareId, const std::string& requestingUserId) {
    for (auto it = m_shares.begin(); it != m_shares.end(); ++it) {
        if (it->id == shareId) {
            // 验证请求用户是否有权限删除此共享
            // 可以是所有者或管理员
            bool isOwner = (it->ownerId == requestingUserId);
            bool canManageUsers = m_userService->hasPermission(requestingUserId, Permission::ManageUsers);
            
            if (isOwner || canManageUsers) {
                m_shares.erase(it);
                return true;
            }
            
            return false; // 无权限
        }
    }
    
    return false; // 共享不存在
}

bool DataSharingService::canAccessData(const std::string& shareId, const std::string& userId) {
    auto sharePtr = getShare(shareId);
    if (!sharePtr) {
        return false; // 共享不存在
    }
    
    // 检查是否过期
    if (isShareExpired(*sharePtr)) {
        return false;
    }
    
    // 检查是否是所有者或被共享的用户
    if (sharePtr->ownerId == userId || sharePtr->sharedWithUserId == userId) {
        return true;
    }
    
    // 检查是否是公共共享
    if (sharePtr->sharedWithUserId.empty()) {
        return true;
    }
    
    return false; // 无访问权限
}

std::string DataSharingService::generateShareToken() {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, chars.length() - 1);
    
    std::string token;
    token.reserve(16);
    
    for (int i = 0; i < 16; ++i) {
        token += chars[distribution(generator)];
    }
    
    return token;
}

int DataSharingService::cleanupExpiredShares() {
    int cleanedCount = 0;
    
    for (auto it = m_shares.begin(); it != m_shares.end(); ) {
        if (isShareExpired(*it)) {
            it = m_shares.erase(it);
            cleanedCount++;
        } else {
            ++it;
        }
    }
    
    return cleanedCount;
}

// 私有方法实现
std::chrono::system_clock::time_point DataSharingService::calculateExpiryTime(
    const std::chrono::system_clock::time_point& createdAt,
    ShareExpiry expiry) {
    
    using namespace std::chrono;
    
    switch (expiry) {
        case ShareExpiry::Permanent:
            return system_clock::time_point::max();
            
        case ShareExpiry::OneDay:
            return createdAt + hours(24);
            
        case ShareExpiry::SevenDays:
            return createdAt + hours(24 * 7);
            
        case ShareExpiry::ThirtyDays:
            return createdAt + hours(24 * 30);
            
        case ShareExpiry::NinetyDays:
            return createdAt + hours(24 * 90);
            
        default:
            return createdAt + hours(24 * 7); // 默认7天
    }
}

bool DataSharingService::isShareExpired(const ShareRecord& share) {
    auto now = std::chrono::system_clock::now();
    return now >= share.expiresAt;
}

void DataSharingService::updateShareAccessStats(const std::string& shareId) {
    for (auto& share : m_shares) {
        if (share.id == shareId) {
            share.downloadCount++;
            share.lastAccessedAt = std::chrono::system_clock::now();
            break;
        }
    }
}

// SharingUtils 实现
std::string SharingUtils::permissionToString(SharePermission permission) {
    switch (permission) {
        case SharePermission::ReadOnly: return "Read Only";
        case SharePermission::ReadWrite: return "Read/Write";
        case SharePermission::Manage: return "Manage";
        default: return "Unknown";
    }
}

std::string SharingUtils::expiryToString(ShareExpiry expiry) {
    switch (expiry) {
        case ShareExpiry::Permanent: return "Permanent";
        case ShareExpiry::OneDay: return "1 Day";
        case ShareExpiry::SevenDays: return "7 Days";
        case ShareExpiry::ThirtyDays: return "30 Days";
        case ShareExpiry::NinetyDays: return "90 Days";
        default: return "Unknown";
    }
}

std::string SharingUtils::formatTimeRemaining(
    const std::chrono::system_clock::time_point& expiresAt) {
    
    auto now = std::chrono::system_clock::now();
    if (now >= expiresAt) {
        return "Expired";
    }
    
    auto remaining = std::chrono::duration_cast<std::chrono::hours>(expiresAt - now).count();
    
    if (remaining < 24) {
        return std::to_string(remaining) + " hours";
    } else if (remaining < 24 * 7) {
        return std::to_string(remaining / 24) + " days";
    } else if (remaining < 24 * 30) {
        return std::to_string(remaining / (24 * 7)) + " weeks";
    } else {
        return std::to_string(remaining / (24 * 30)) + " months";
    }
}

bool SharingUtils::permissionAllowsOperation(
    SharePermission permission,
    const std::string& operation) {
    
    switch (permission) {
        case SharePermission::ReadOnly:
            return operation == "read";
            
        case SharePermission::ReadWrite:
            return operation == "read" || operation == "write";
            
        case SharePermission::Manage:
            return operation == "read" || operation == "write" || operation == "delete";
            
        default:
            return false;
    }
}

} // namespace Collaboration
} // namespace Core
} // namespace BondForge