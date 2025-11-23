#include "PermissionManager.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>

namespace BondForge {
namespace Core {
namespace Permissions {

// PermissionManager 实现
PermissionManager::PermissionManager(std::unique_ptr<Collaboration::IUserService> userService)
    : m_userService(std::move(userService)) {
    
    // 初始化默认策略和属性
    initializeDefaultPolicies();
    initializeDefaultAttributes();
}

bool PermissionManager::hasPermission(const std::string& userId, Collaboration::Permission permission) {
    // 使用用户服务检查RBAC权限
    return m_userService->hasPermission(userId, permission);
}

AccessDecision PermissionManager::checkAccess(const AccessRequest& request) {
    AccessDecision decision;
    decision.permitted = false;
    
    // 首先检查基于RBAC的基本权限
    Collaboration::Permission rbacPermission;
    
    // 将操作映射到权限枚举
    if (request.action == "read") {
        rbacPermission = Collaboration::Permission::Read;
    } else if (request.action == "write") {
        rbacPermission = Collaboration::Permission::Write;
    } else if (request.action == "delete") {
        rbacPermission = Collaboration::Permission::Delete;
    } else if (request.action == "share") {
        rbacPermission = Collaboration::Permission::Share;
    } else {
        decision.reason = "Unknown action: " + request.action;
        return decision;
    }
    
    // 检查RBAC权限
    if (!hasPermission(request.subjectId, rbacPermission)) {
        decision.reason = "RBAC: User does not have the required permission";
        return decision;
    }
    
    // 检查ABAC策略
    std::vector<AccessPolicy> applicablePolicies;
    for (const auto& policy : m_policies) {
        if (!policy.isActive) continue;
        
        // 检查策略是否适用于当前请求
        bool appliesToResource = attributesMatch(policy.resourceAttributes, request.resourceAttributes);
        bool appliesToSubject = attributesMatch(policy.subjectAttributes, request.subjectAttributes);
        bool appliesToEnvironment = attributesMatch(policy.environmentAttributes, request.environmentAttributes);
        
        if (appliesToResource && appliesToSubject && appliesToEnvironment) {
            applicablePolicies.push_back(policy);
        }
    }
    
    // 如果没有适用的策略，默认拒绝
    if (applicablePolicies.empty()) {
        decision.reason = "ABAC: No applicable policies found";
        return decision;
    }
    
    // 检查是否有策略允许该操作
    for (const auto& policy : applicablePolicies) {
        if (std::find(policy.allowedActions.begin(), policy.allowedActions.end(), request.action) 
            != policy.allowedActions.end()) {
            decision.permitted = true;
            decision.policyId = policy.id;
            decision.reason = "Allowed by policy: " + policy.name;
            
            // 添加日志记录义务
            Attribute logObligation = AttributeUtils::createAttribute("log", "true");
            decision.obligations.push_back(logObligation);
            break;
        }
    }
    
    if (!decision.permitted) {
        decision.reason = "ABAC: No policy allows this action";
    }
    
    // 记录访问决策
    logAccessDecision(decision, request);
    
    return decision;
}

bool PermissionManager::addPolicy(const AccessPolicy& policy) {
    // 检查策略ID是否已存在
    for (const auto& existingPolicy : m_policies) {
        if (existingPolicy.id == policy.id) {
            return false; // 策略ID已存在
        }
    }
    
    m_policies.push_back(policy);
    return true;
}

bool PermissionManager::updatePolicy(const AccessPolicy& policy) {
    for (auto& existingPolicy : m_policies) {
        if (existingPolicy.id == policy.id) {
            existingPolicy = policy;
            return true;
        }
    }
    
    return false; // 策略不存在
}

bool PermissionManager::deletePolicy(const std::string& policyId) {
    for (auto it = m_policies.begin(); it != m_policies.end(); ++it) {
        if (it->id == policyId) {
            m_policies.erase(it);
            return true;
        }
    }
    
    return false; // 策略不存在
}

std::vector<AccessPolicy> PermissionManager::getAllPolicies() {
    return m_policies;
}

std::vector<AccessPolicy> PermissionManager::getActivePolicies() {
    std::vector<AccessPolicy> activePolicies;
    for (const auto& policy : m_policies) {
        if (policy.isActive) {
            activePolicies.push_back(policy);
        }
    }
    return activePolicies;
}

bool PermissionManager::addUserAttribute(const std::string& userId, const Attribute& attribute) {
    auto& attributes = m_userAttributes[userId];
    
    // 检查属性是否已存在
    for (const auto& existingAttr : attributes) {
        if (existingAttr.name == attribute.name) {
            return false; // 属性已存在
        }
    }
    
    attributes.push_back(attribute);
    return true;
}

bool PermissionManager::removeUserAttribute(const std::string& userId, const std::string& attributeName) {
    auto it = m_userAttributes.find(userId);
    if (it == m_userAttributes.end()) {
        return false; // 用户不存在
    }
    
    auto& attributes = it->second;
    for (auto attrIt = attributes.begin(); attrIt != attributes.end(); ++attrIt) {
        if (attrIt->name == attributeName) {
            attributes.erase(attrIt);
            return true;
        }
    }
    
    return false; // 属性不存在
}

std::vector<Attribute> PermissionManager::getUserAttributes(const std::string& userId) {
    auto it = m_userAttributes.find(userId);
    if (it != m_userAttributes.end()) {
        return it->second;
    }
    return {};
}

bool PermissionManager::addResourceAttribute(const std::string& resourceId, const Attribute& attribute) {
    auto& attributes = m_resourceAttributes[resourceId];
    
    // 检查属性是否已存在
    for (const auto& existingAttr : attributes) {
        if (existingAttr.name == attribute.name) {
            return false; // 属性已存在
        }
    }
    
    attributes.push_back(attribute);
    return true;
}

bool PermissionManager::removeResourceAttribute(const std::string& resourceId, const std::string& attributeName) {
    auto it = m_resourceAttributes.find(resourceId);
    if (it == m_resourceAttributes.end()) {
        return false; // 资源不存在
    }
    
    auto& attributes = it->second;
    for (auto attrIt = attributes.begin(); attrIt != attributes.end(); ++attrIt) {
        if (attrIt->name == attributeName) {
            attributes.erase(attrIt);
            return true;
        }
    }
    
    return false; // 属性不存在
}

std::vector<Attribute> PermissionManager::getResourceAttributes(const std::string& resourceId) {
    auto it = m_resourceAttributes.find(resourceId);
    if (it != m_resourceAttributes.end()) {
        return it->second;
    }
    return {};
}

// 私有方法实现
bool PermissionManager::evaluatePolicy(const AccessPolicy& policy, const AccessRequest& request) {
    // 检查主体属性
    if (!attributesMatch(policy.subjectAttributes, request.subjectAttributes)) {
        return false;
    }
    
    // 检查资源属性
    if (!attributesMatch(policy.resourceAttributes, request.resourceAttributes)) {
        return false;
    }
    
    // 检查环境属性
    if (!attributesMatch(policy.environmentAttributes, request.environmentAttributes)) {
        return false;
    }
    
    // 检查操作是否被允许
    return std::find(policy.allowedActions.begin(), policy.allowedActions.end(), request.action) 
           != policy.allowedActions.end();
}

bool PermissionManager::attributesMatch(
    const std::vector<Attribute>& policyAttributes,
    const std::vector<Attribute>& requestAttributes) {
    
    // 如果策略没有指定属性，则默认匹配
    if (policyAttributes.empty()) {
        return true;
    }
    
    // 检查请求是否包含所有策略属性
    for (const auto& policyAttr : policyAttributes) {
        bool found = false;
        for (const auto& requestAttr : requestAttributes) {
            if (policyAttr.name == requestAttr.name && 
                policyAttr.value == requestAttr.value) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            return false;
        }
    }
    
    return true;
}

void PermissionManager::initializeDefaultPolicies() {
    // 策略1: 工作时间访问策略
    AccessPolicy workingHoursPolicy;
    workingHoursPolicy.id = "policy-working-hours";
    workingHoursPolicy.name = "Working Hours Access";
    workingHoursPolicy.description = "Allow access only during working hours (9am-5pm)";
    workingHoursPolicy.environmentAttributes.push_back(
        AttributeUtils::createAttribute("time_range", "09:00-17:00")
    );
    workingHoursPolicy.allowedActions = {"read", "write", "delete", "share"};
    workingHoursPolicy.isActive = true;
    
    // 策略2: 敏感数据访问策略
    AccessPolicy sensitiveDataPolicy;
    sensitiveDataPolicy.id = "policy-sensitive-data";
    sensitiveDataPolicy.name = "Sensitive Data Access";
    sensitiveDataPolicy.description = "Restrict access to sensitive data to researchers and above";
    sensitiveDataPolicy.resourceAttributes.push_back(
        AttributeUtils::createAttribute("sensitivity", "high")
    );
    sensitiveDataPolicy.subjectAttributes.push_back(
        AttributeUtils::createAttribute("min_role", "Researcher")
    );
    sensitiveDataPolicy.allowedActions = {"read"};
    sensitiveDataPolicy.isActive = true;
    
    // 策略3: 数据所有者完全访问策略
    AccessPolicy ownerAccessPolicy;
    ownerAccessPolicy.id = "policy-owner-access";
    ownerAccessPolicy.name = "Owner Full Access";
    ownerAccessPolicy.description = "Data owners have full access to their data";
    ownerAccessPolicy.resourceAttributes.push_back(
        AttributeUtils::createAttribute("ownership", "self")
    );
    ownerAccessPolicy.allowedActions = {"read", "write", "delete", "share"};
    ownerAccessPolicy.isActive = true;
    
    m_policies.push_back(workingHoursPolicy);
    m_policies.push_back(sensitiveDataPolicy);
    m_policies.push_back(ownerAccessPolicy);
}

void PermissionManager::initializeDefaultAttributes() {
    // 添加默认用户属性
    std::vector<std::string> userIds = {
        "admin-001", "user-researcher-001", "user-analyst-001", "user-guest-001"
    };
    
    for (const auto& userId : userIds) {
        std::vector<Attribute> userAttrs;
        
        // 根据用户ID设置默认角色属性
        if (userId == "admin-001") {
            userAttrs.push_back(AttributeUtils::createAttribute("min_role", "Admin"));
            userAttrs.push_back(AttributeUtils::createAttribute("department", "IT"));
        } else if (userId == "user-researcher-001") {
            userAttrs.push_back(AttributeUtils::createAttribute("min_role", "Researcher"));
            userAttrs.push_back(AttributeUtils::createAttribute("department", "Chemistry"));
        } else if (userId == "user-analyst-001") {
            userAttrs.push_back(AttributeUtils::createAttribute("min_role", "Analyst"));
            userAttrs.push_back(AttributeUtils::createAttribute("department", "Data Analysis"));
        } else if (userId == "user-guest-001") {
            userAttrs.push_back(AttributeUtils::createAttribute("min_role", "Guest"));
            userAttrs.push_back(AttributeUtils::createAttribute("department", "External"));
        }
        
        m_userAttributes[userId] = userAttrs;
    }
}

void PermissionManager::logAccessDecision(const AccessDecision& decision, const AccessRequest& request) {
    // 在实际应用中，这里应该写入日志文件或数据库
    // 为简化，仅输出到控制台
    std::cout << "[" << std::chrono::system_clock::now().time_since_epoch().count() << "] ";
    std::cout << "ACCESS_DECISION: " << (decision.permitted ? "PERMIT" : "DENY");
    std::cout << " | User: " << request.subjectId;
    std::cout << " | Resource: " << request.resourceId;
    std::cout << " | Action: " << request.action;
    std::cout << " | Policy: " << decision.policyId;
    std::cout << " | Reason: " << decision.reason;
    std::cout << std::endl;
}

// AttributeUtils 实现
Attribute AttributeUtils::createAttribute(const std::string& name, const std::string& value) {
    Attribute attr;
    attr.name = name;
    attr.value = value;
    return attr;
}

std::string AttributeUtils::attributesToString(const std::vector<Attribute>& attributes) {
    std::stringstream ss;
    ss << "{";
    
    for (size_t i = 0; i < attributes.size(); ++i) {
        ss << "\"" << attributes[i].name << "\":\"" << attributes[i].value << "\"";
        if (i < attributes.size() - 1) {
            ss << ",";
        }
    }
    
    ss << "}";
    return ss.str();
}

std::vector<Attribute> AttributeUtils::parseAttributes(const std::string& str) {
    std::vector<Attribute> attributes;
    
    // 简化实现，实际应用中应使用更健壮的JSON解析器
    if (str.empty() || str[0] != '{' || str.back() != '}') {
        return attributes;
    }
    
    // 移除大括号
    std::string content = str.substr(1, str.length() - 2);
    
    // 简单分割（不处理嵌套和转义）
    std::stringstream ss(content);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        size_t colonPos = token.find(':');
        if (colonPos != std::string::npos) {
            std::string name = token.substr(0, colonPos);
            std::string value = token.substr(colonPos + 1);
            
            // 移除引号
            if (name.length() >= 2 && name.front() == '"' && name.back() == '"') {
                name = name.substr(1, name.length() - 2);
            }
            if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            
            attributes.push_back(createAttribute(name, value));
        }
    }
    
    return attributes;
}

} // namespace Permissions
} // namespace Core
} // namespace BondForge