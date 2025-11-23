#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H

#include "../../core/plugins/PluginInterface.h"
#include <QObject>
#include <QtPlugin>
#include <QJsonObject>

class ExamplePlugin : public QObject, public BondForge::Core::Plugins::PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "BondForge.ExamplePlugin" FILE "metadata.json")
    Q_INTERFACES(BondForge::Core::Plugins::PluginInterface)

public:
    ExamplePlugin(QObject *parent = nullptr);
    ~ExamplePlugin() override;

    // PluginInterface interface
    QString name() const override;
    QString version() const override;
    QString description() const override;
    QString author() const override;
    QString website() const override;
    QStringList dependencies() const override;
    bool initialize(BondForge::Core::Plugins::PluginContext *context) override;
    void shutdown() override;
    void configure() override;
    void execute(const QString &command, const QVariantMap &parameters) override;
    QVariantMap executeWithReturn(const QString &command, const QVariantMap &parameters) override;
    bool isExecutable(const QString &command) const override;
    QStringList commands() const override;
    QJsonObject getMetadata() const override;

private slots:
    void onMenuAction();
    void processQueue();

private:
    void setupUI();
    void registerCommands();
    
    // Plugin specific methods
    QString calculateMolecularWeight(const QString& formula);
    QString predictSolubility(const QVariantMap& properties);
    QString generateMoleculeImage(const QString& smiles, const QString& format);
    
    BondForge::Core::Plugins::PluginContext *m_context;
    QString m_name;
    QString m_version;
    QString m_description;
    QString m_author;
    QString m_website;
    QStringList m_dependencies;
    bool m_initialized;
    QJsonObject m_metadata;
};

#endif // EXAMPLEPLUGIN_H