#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include "../collaboration/User.h"

namespace BondForge {
namespace Core {
namespace Permissions {

/**
 * @brief 属性基访问控制(ABAC)属性
 */
struct Attribute {
    std::string name;   // 属性名
    std::string value;  // 属性值
};

/**
 * @brief 访问控制策略
 */
struct AccessPolicy {
    std::string id;                          // 策略ID
    std::string name;                        // 策略名称
    std::string description;                 // 策略描述
    std::vector<Attribute> subjectAttributes;   // 主体属性条件
    std::vector<Attribute> resourceAttributes;  // 资源属性条件
    std::vector<Attribute> environmentAttributes; // 环境属性条件
    std::vector<std::string> allowedActions;  // 允许的操作
    bool isActive;                              // 是否激活
};

/**
 * @brief 访问请求
 */
struct AccessRequest {
    std::string subjectId;                  // 主体ID（通常是用户ID）
    std::vector<Attribute> subjectAttributes;  // 主体属性
    std::string resourceId;                  // 资源ID
    std::vector<Attribute> resourceAttributes; // 资源属性
    std::string action;                     // 请求的操作
    std::vector<Attribute> environmentAttributes; // 环境属性
    std::string context;                    // 上下文信息
    std::chrono::system_clock::time_point timestamp; // 请求时间
};

/**
 * @brief 访问决策
 */
struct AccessDecision {
    bool permitted;                         // 是否允许
    std::string policyId;                  // 应用的策略ID
    std::string reason;                      // 决策原因
    std::vector<Attribute> obligations;     // 义务（例如需要记录日志）
};

/**
 * @brief 权限管理器接口
 */
class IPermissionManager {
public:
    virtual ~IPermissionManager() = default;
    
    /**
     * @brief 检查访问权限（基于RBAC）
     * 
     * @param userId 用户ID
     * @param permission 权限
     * @return 是否有权限
     */
    virtual bool hasPermission(const std::string& userId, Collaboration::Permission permission) = 0;
    
    /**
     * @brief 检查访问权限（基于ABAC）
     * 
     * @param request 访问请求
     * @return 访问决策
     */
    virtual AccessDecision checkAccess(const AccessRequest& request) = 0;
    
    /**
     * @brief 添加访问控制策略
     * 
     * @param policy 策略
     * @return 是否成功
     */
    virtual bool addPolicy(const AccessPolicy& policy) = 0;
    
    /**
     * @brief 更新访问控制策略
     * 
     * @param policy 策略
     * @return 是否成功
     */
    virtual bool updatePolicy(const AccessPolicy& policy) = 0;
    
    /**
     * @brief 删除访问控制策略
     * 
     * @param policyId 策略ID
     * @return 是否成功
     */
    virtual bool deletePolicy(const std::string& policyId) = 0;
    
    /**
     * @brief 获取所有策略
     * 
     * @return 策略列表
     */
    virtual std::vector<AccessPolicy> getAllPolicies() = 0;
    
    /**
     * @brief 获取激活的策略
     * 
     * @return 激活的策略列表
     */
    virtual std::vector<AccessPolicy> getActivePolicies() = 0;
};

/**
 * @brief 权限管理器实现类
 * 
 * 结合RBAC和ABAC的混合权限系统
 */
class PermissionManager : public IPermissionManager {
private:
    std::unique_ptr<Collaboration::IUserService> m_userService;
    std::vector<AccessPolicy> m_policies;
    std::map<std::string, std::vector<Attribute>> m_userAttributes;  // userId -> attributes
    std::map<std::string, std::vector<Attribute>> m_resourceAttributes;  // resourceId -> attributes
    
    /**
     * @brief 评估单个策略
     * 
     * @param policy 策略
     * @param request 访问请求
     * @return 是否匹配
     */
    bool evaluatePolicy(const AccessPolicy& policy, const AccessRequest& request);
    
    /**
     * @brief 检查属性是否匹配
     * 
     * @param policyAttributes 策略属性
     * @param requestAttributes 请求属性
     * @return 是否匹配
     */
    bool attributesMatch(
        const std::vector<Attribute>& policyAttributes,
        const std::vector<Attribute>& requestAttributes);
    
    /**
     * @brief 初始化默认策略
     */
    void initializeDefaultPolicies();
    
    /**
     * @brief 初始化默认用户和资源属性
     */
    void initializeDefaultAttributes();
    
    /**
     * @brief 记录访问决策日志
     * 
     * @param decision 访问决策
     * @param request 访问请求
     */
    void logAccessDecision(const AccessDecision& decision, const AccessRequest& request);
    
public:
    explicit PermissionManager(std::unique_ptr<Collaboration::IUserService> userService);
    
    bool hasPermission(const std::string& userId, Collaboration::Permission permission) override;
    AccessDecision checkAccess(const AccessRequest& request) override;
    bool addPolicy(const AccessPolicy& policy) override;
    bool updatePolicy(const AccessPolicy& policy) override;
    bool deletePolicy(const std::string& policyId) override;
    std::vector<AccessPolicy> getAllPolicies() override;
    std::vector<AccessPolicy> getActivePolicies() override;
    
    // 属性管理方法
    bool addUserAttribute(const std::string& userId, const Attribute& attribute);
    bool removeUserAttribute(const std::string& userId, const std::string& attributeName);
    std::vector<Attribute> getUserAttributes(const std::string& userId);
    
    bool addResourceAttribute(const std::string& resourceId, const Attribute& attribute);
    bool removeResourceAttribute(const std::string& resourceId, const std::string& attributeName);
    std::vector<Attribute> getResourceAttributes(const std::string& resourceId);
};

/**
 * @brief 属性工具
 */
class AttributeUtils {
public:
    /**
     * @brief 创建属性
     * 
     * @param name 属性名
     * @param value 属性值
     * @return 属性对象
     */
    static Attribute createAttribute(const std::string& name, const std::string& value);
    
    /**
     * @brief 将属性转换为字符串
     * 
     * @param attributes 属性列表
     * @return 字符串表示
     */
    static std::string attributesToString(const std::vector<Attribute>& attributes);
    
    /**
     * @brief 从字符串解析属性
     * 
     * @param str 字符串表示
     * @return 属性列表
     */
    static std::vector<Attribute> parseAttributes(const std::string& str);
};

} // namespace Permissions
} // namespace Core
} // namespace BondForge