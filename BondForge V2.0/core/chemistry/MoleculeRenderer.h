#pragma once

#include <QGraphicsScene>
#include <QPixmap>
#include <memory>
#include "../data/DataRecord.h"

#ifdef USE_RDKIT
#include <GraphMol/MolDraw2D/MolDraw2DQt.h>
#include <GraphMol/ROMol.h>
#endif

namespace BondForge {
namespace Core {
namespace Chemistry {

/**
 * @brief 分子渲染器接口
 * 
 * 定义分子结构渲染的通用接口
 */
class IMoleculeRenderer {
public:
    virtual ~IMoleculeRenderer() = default;
    
    /**
     * @brief 渲染分子结构
     * 
     * @param scene QGraphics场景，用于显示渲染结果
     * @param record 包含分子数据的记录
     * @param is3D 是否使用3D渲染
     * @return 是否成功渲染
     */
    virtual bool renderMolecule(
        QGraphicsScene* scene, 
        const Data::DataRecord& record, 
        bool is3D = false) = 0;
        
    /**
     * @brief 检查是否支持指定的格式
     * 
     * @param format 数据格式
     * @return 是否支持
     */
    virtual bool supportsFormat(const std::string& format) const = 0;
    
    /**
     * @brief 获取渲染器名称
     * 
     * @return 渲染器名称
     */
    virtual std::string getRendererName() const = 0;
};

/**
 * @brief 简化的分子渲染器实现
 * 
 * 使用Qt内置图形功能实现基础的分子可视化
 */
class SimpleMoleculeRenderer : public IMoleculeRenderer {
public:
    bool renderMolecule(
        QGraphicsScene* scene, 
        const Data::DataRecord& record, 
        bool is3D = false) override;
        
    bool supportsFormat(const std::string& format) const override;
    std::string getRendererName() const override { return "Simple Renderer"; }
    
private:
    void renderSimpleBenzene(QGraphicsScene* scene, bool is3D);
    void addMoleculeLegend(QGraphicsScene* scene, bool is3D);
};

#ifdef USE_RDKIT
/**
 * @brief 基于RDKit的专业分子渲染器
 * 
 * 使用RDKit库实现专业的分子结构可视化
 */
class RDKitMoleculeRenderer : public IMoleculeRenderer {
public:
    bool renderMolecule(
        QGraphicsScene* scene, 
        const Data::DataRecord& record, 
        bool is3D = false) override;
        
    bool supportsFormat(const std::string& format) const override;
    std::string getRendererName() const override { return "RDKit Renderer"; }
    
private:
    std::unique_ptr<RDKit::ROMol> parseMolecule(const std::string& content);
    void renderMolecule2D(QGraphicsScene* scene, RDKit::ROMol* mol);
    void renderMolecule3D(QGraphicsScene* scene, RDKit::ROMol* mol);
};
#endif

/**
 * @brief 分子渲染器工厂
 * 
 * 根据可用库和数据格式创建合适的渲染器
 */
class MoleculeRendererFactory {
public:
    /**
     * @brief 创建最佳的分子渲染器
     * 
     * @param format 数据格式
     * @return 渲染器实例
     */
    static std::unique_ptr<IMoleculeRenderer> createRenderer(const std::string& format = "");
    
    /**
     * @brief 获取所有可用的渲染器名称
     * 
     * @return 渲染器名称列表
     */
    static std::vector<std::string> getAvailableRenderers();
};

} // namespace Chemistry
} // namespace Core
} // namespace BondForge