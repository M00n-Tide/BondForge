#include "MoleculeRenderer.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPainter>
#include <sstream>

namespace BondForge {
namespace Core {
namespace Chemistry {

// SimpleMoleculeRenderer 实现
bool SimpleMoleculeRenderer::renderMolecule(
    QGraphicsScene* scene, 
    const Data::DataRecord& record, 
    bool is3D) {
    
    if (record.content.empty()) {
        scene->addText("No molecular structure data");
        return false;
    }
    
    // 清除现有内容
    scene->clear();
    
    // 简化的分子渲染（仅支持演示用）
    // 检测内容是否包含碳原子标识符，如果是则渲染一个简单的苯环结构
    if (record.content.find("C") != std::string::npos || record.content.find("c") != std::string::npos) {
        renderSimpleBenzene(scene, is3D);
    } else {
        // 否则仅显示内容文本
        scene->addText(QString::fromStdString(record.content), QFont("Arial", 14, QFont::Bold));
    }
    
    // 添加分子信息
    QGraphicsTextItem* info = scene->addText(
        QString("ID: %1 | Format: %2 | Category: %3")
        .arg(QString::fromStdString(record.id))
        .arg(QString::fromStdString(record.format))
        .arg(QString::fromStdString(record.category)),
        QFont("Arial", 10)
    );
    info->setPos(10, 10);
    
    // 添加图例
    addMoleculeLegend(scene, is3D);
    
    return true;
}

bool SimpleMoleculeRenderer::supportsFormat(const std::string& format) const {
    // 简单渲染器支持所有格式（作为后备选项）
    return true;
}

void SimpleMoleculeRenderer::renderSimpleBenzene(QGraphicsScene* scene, bool is3D) {
    const int centerX = 350;
    const int centerY = 250;
    const int radius = 80;
    
    if (is3D) {
        // 3D视图 - 使用椭圆表示原子，线条表示化学键
        QPen bondPen(Qt::black, 3);
        QBrush carbonBrush(QColor(50, 50, 50));
        QBrush hydrogenBrush(QColor(200, 200, 200));
        
        // 绘制六元环
        QGraphicsEllipseItem* carbon1 = scene->addEllipse(centerX - radius - 15, centerY - 15, 30, 30, QPen(), carbonBrush);
        QGraphicsEllipseItem* carbon2 = scene->addEllipse(centerX + radius - 15, centerY - 15, 30, 30, QPen(), carbonBrush);
        QGraphicsEllipseItem* carbon3 = scene->addEllipse(centerX + radius - 15, centerY + radius - 15, 30, 30, QPen(), carbonBrush);
        QGraphicsEllipseItem* carbon4 = scene->addEllipse(centerX - 15, centerY + radius - 15, 30, 30, QPen(), carbonBrush);
        QGraphicsEllipseItem* carbon5 = scene->addEllipse(centerX - radius - 15, centerY + radius - 15, 30, 30, QPen(), carbonBrush);
        QGraphicsEllipseItem* carbon6 = scene->addEllipse(centerX - radius - 15, centerY - radius - 15, 30, 30, QPen(), carbonBrush);
        
        // 绘制化学键
        scene->addLine(centerX - radius, centerY, centerX, centerY - radius, bondPen);
        scene->addLine(centerX, centerY - radius, centerX + radius, centerY, bondPen);
        scene->addLine(centerX + radius, centerY, centerX, centerY + radius, bondPen);
        scene->addLine(centerX, centerY + radius, centerX - radius, centerY, bondPen);
        scene->addLine(centerX - radius, centerY, centerX - radius, centerY - radius, bondPen);
        scene->addLine(centerX - radius, centerY - radius, centerX, centerY - radius, bondPen);
        
        // 添加氢原子（简化）
        // ...（省略详细的氢原子绘制代码）
    } else {
        // 2D视图 - 使用圆形表示原子，线条表示化学键
        QPen bondPen(Qt::black, 2);
        QBrush carbonBrush(Qt::black);
        
        // 绘制六元环
        QGraphicsEllipseItem* carbon1 = scene->addEllipse(centerX - radius - 10, centerY - 10, 20, 20, QPen(Qt::black), carbonBrush);
        QGraphicsEllipseItem* carbon2 = scene->addEllipse(centerX + radius - 10, centerY - 10, 20, 20, QPen(Qt::black), carbonBrush);
        QGraphicsEllipseItem* carbon3 = scene->addEllipse(centerX + radius - 10, centerY + radius - 10, 20, 20, QPen(Qt::black), carbonBrush);
        QGraphicsEllipseItem* carbon4 = scene->addEllipse(centerX - 10, centerY + radius - 10, 20, 20, QPen(Qt::black), carbonBrush);
        QGraphicsEllipseItem* carbon5 = scene->addEllipse(centerX - radius - 10, centerY + radius - 10, 20, 20, QPen(Qt::black), carbonBrush);
        QGraphicsEllipseItem* carbon6 = scene->addEllipse(centerX - radius - 10, centerY - radius - 10, 20, 20, QPen(Qt::black), carbonBrush);
        
        // 绘制化学键
        scene->addLine(centerX - radius, centerY, centerX, centerY - radius, bondPen);
        scene->addLine(centerX, centerY - radius, centerX + radius, centerY, bondPen);
        scene->addLine(centerX + radius, centerY, centerX, centerY + radius, bondPen);
        scene->addLine(centerX, centerY + radius, centerX - radius, centerY, bondPen);
        scene->addLine(centerX - radius, centerY, centerX - radius, centerY - radius, bondPen);
        scene->addLine(centerX - radius, centerY - radius, centerX, centerY - radius, bondPen);
    }
}

void SimpleMoleculeRenderer::addMoleculeLegend(QGraphicsScene* scene, bool is3D) {
    // 添加图例
    QGroupBox* legendGroup = new QGroupBox();
    legendGroup->setTitle("Legend");
    QVBoxLayout* legendLayout = new QVBoxLayout(legendGroup);
    
    QGraphicsProxyWidget* proxy = scene->addWidget(legendGroup);
    proxy->setPos(550, 30);
    
    QBrush carbonBrush = is3D ? QBrush(QColor(50, 50, 50)) : QBrush(Qt::black);
    QBrush hydrogenBrush = is3D ? QBrush(QColor(200, 200, 200)) : QBrush(Qt::white);
    
    // 添加图例项
    QGraphicsEllipseItem* carbonLegend = scene->addEllipse(0, 0, 10, 10, QPen(), carbonBrush);
    QGraphicsTextItem* carbonText = scene->addText("C (Carbon)", QFont("Arial", 9));
    
    QGraphicsEllipseItem* hydrogenLegend = scene->addEllipse(0, 20, 10, 10, QPen(), hydrogenBrush);
    QGraphicsTextItem* hydrogenText = scene->addText("H (Hydrogen)", QFont("Arial", 9));
    
    // 碳图例
    carbonLegend->setPos(560, 60);
    carbonText->setPos(580, 55);
    
    // 氢图例
    hydrogenLegend->setPos(560, 80);
    hydrogenText->setPos(580, 75);
}

#ifdef USE_RDKIT
// RDKitMoleculeRenderer 实现
std::unique_ptr<RDKit::ROMol> RDKitMoleculeRenderer::parseMolecule(const std::string& content) {
    try {
        // 尝试解析不同格式的分子数据
        std::unique_ptr<RDKit::ROMol> mol;
        
        // 1. 尝试解析为SMILES
        try {
            mol.reset(RDKit::SmilesToMol(content));
        } catch (...) {
            // 2. 尝试解析为SDF/MOL文件内容
            std::istringstream iss(content);
            try {
                mol.reset(RDKit::MolDataStreamToMol(iss));
            } catch (...) {
                // 3. 解析失败
                return nullptr;
            }
        }
        
        return mol;
    } catch (...) {
        return nullptr;
    }
}

bool RDKitMoleculeRenderer::renderMolecule(
    QGraphicsScene* scene, 
    const Data::DataRecord& record, 
    bool is3D) {
    
    if (record.content.empty()) {
        scene->addText("No molecular structure data");
        return false;
    }
    
    // 清除现有内容
    scene->clear();
    
    // 解析分子
    auto mol = parseMolecule(record.content);
    if (!mol) {
        scene->addText(QString("无法解析分子结构: %1").arg(QString::fromStdString(record.content)));
        return false;
    }
    
    // 根据是否3D选择渲染方式
    if (is3D) {
        renderMolecule3D(scene, mol.get());
    } else {
        renderMolecule2D(scene, mol.get());
    }
    
    // 添加分子信息
    QGraphicsTextItem* info = scene->addText(
        QString("ID: %1 | Format: %2 | Category: %3")
        .arg(QString::fromStdString(record.id))
        .arg(QString::fromStdString(record.format))
        .arg(QString::fromStdString(record.category)),
        QFont("Arial", 10)
    );
    info->setPos(10, 10);
    
    return true;
}

bool RDKitMoleculeRenderer::supportsFormat(const std::string& format) const {
    // RDKit支持专业化学格式
    return format == "SDF" || format == "MOL" || format == "SMILES";
}

void RDKitMoleculeRenderer::renderMolecule2D(QGraphicsScene* scene, RDKit::ROMol* mol) {
    // 创建绘图对象
    int width = 700, height = 500;
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    
    RDKit::MolDraw2DQt drawer(width, height, &painter);
    drawer.drawMolecule(*mol);
    
    // 将结果添加到场景
    QGraphicsPixmapItem* item = scene->addPixmap(pixmap);
    scene->setSceneRect(item->boundingRect());
}

void RDKitMoleculeRenderer::renderMolecule3D(QGraphicsScene* scene, RDKit::ROMol* mol) {
    // 3D渲染实现（简化版）
    // 实际应用中可以使用更高级的3D渲染
    renderMolecule2D(scene, mol);  // 回退到2D渲染
}
#endif

// MoleculeRendererFactory 实现
std::unique_ptr<IMoleculeRenderer> MoleculeRendererFactory::createRenderer(const std::string& format) {
#ifdef USE_RDKIT
    // 优先使用RDKit（如果可用且支持指定格式）
    auto rdkitRenderer = std::make_unique<RDKitMoleculeRenderer>();
    if (rdkitRenderer->supportsFormat(format)) {
        return rdkitRenderer;
    }
#endif
    
    // 回退到简单渲染器
    return std::make_unique<SimpleMoleculeRenderer>();
}

std::vector<std::string> MoleculeRendererFactory::getAvailableRenderers() {
    std::vector<std::string> renderers;
    
    // 简单渲染器总是可用
    renderers.push_back("Simple Renderer");
    
#ifdef USE_RDKIT
    renderers.push_back("RDKit Renderer");
#endif
    
    return renderers;
}

} // namespace Chemistry
} // namespace Core
} // namespace BondForge