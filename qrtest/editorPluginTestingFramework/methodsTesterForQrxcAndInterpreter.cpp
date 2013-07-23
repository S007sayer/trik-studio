#include "methodsTesterForQrxcAndInterpreter.h"
#include "convertingMethods.h"
#include "unifiedStringGenerator.h"
#include "defs.h"
#include "../../qrkernel/exception/exception.h"

#include <QtCore/QDebug>

using namespace editorPluginTestingFramework;
using namespace qReal;

MethodsTesterForQrxcAndInterpreter::MethodsTesterForQrxcAndInterpreter(
		EditorManager* qrxcGeneratedPlugin
		, InterpreterEditorManager* interpreterGeneratedPlugin
		)
{
	mQrxcGeneratedPlugin = qrxcGeneratedPlugin;
	mInterpreterGeneratedPlugin = interpreterGeneratedPlugin;
}

class MethodsTesterForQrxcAndInterpreter::StringGenerator : public UnifiedStringGenerator
{
public:
	void init(EditorManagerInterface *editorManagerInterface) {
		mEditorManagerInterface = editorManagerInterface;
	}

	virtual QString generateString() const {
		QString resultStr = "";

		foreach (QString const &elementOfList, generateList(mEditorManagerInterface)) {
			resultStr += elementOfList + " ";
		}

		return resultStr;
	}

protected:
	virtual QStringList generateList(EditorManagerInterface *editorManagerInterface) const = 0;

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId = Id::rootId()
			, Id const &diagramId = Id::rootId()
			, Id const &elementId = Id::rootId()
			, QString const &propertyName = "") const = 0;

private:
	EditorManagerInterface *mEditorManagerInterface;
};

class MethodsTesterForQrxcAndInterpreter::StringGeneratorForEditors : public MethodsTesterForQrxcAndInterpreter::StringGenerator
{
	virtual QStringList generateList(EditorManagerInterface *editorManagerInterface) const {
		QStringList resultList;

		foreach (Id const &editor, editorManagerInterface->editors()) {
			QString const additionalString = ConvertingMethods::transformateOutput(callMethod(editorManagerInterface, editor), editor);
			resultList.append(additionalString);
			resultList.append("|");
		}
		return resultList;
	}
};

class MethodsTesterForQrxcAndInterpreter::StringGeneratorForDiagrams : public MethodsTesterForQrxcAndInterpreter::StringGenerator
{
	virtual QStringList generateList(EditorManagerInterface *editorManagerInterface) const {
		QStringList resultList;

		foreach (Id const &editor, editorManagerInterface->editors()) {
			foreach (Id const &diagram, editorManagerInterface->diagrams(editor)) {
				QString const additionalString = ConvertingMethods::transformateOutput(callMethod(editorManagerInterface, editor, diagram), diagram);
				resultList.append(additionalString);
				resultList.append("|");
			}
		}
		return resultList;
	}
};

class MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements : public MethodsTesterForQrxcAndInterpreter::StringGenerator
{
	virtual QStringList generateList(EditorManagerInterface *editorManagerInterface) const {
		QStringList resultList;

		foreach (Id const &editor, editorManagerInterface->editors()) {
			foreach (Id const &diagram, editorManagerInterface->diagrams(editor)) {
				foreach (Id const &element, editorManagerInterface->elements(diagram)) {
					QString const additionalString = ConvertingMethods::transformateOutput(callMethod(editorManagerInterface, editor, diagram, element), element);
					resultList.append(additionalString);
					resultList.append("|");
				}
			}
		}
		return resultList;
	}
};

class MethodsTesterForQrxcAndInterpreter::StringGeneratorForProperties : public MethodsTesterForQrxcAndInterpreter::StringGenerator
{
	virtual QStringList generateList(EditorManagerInterface *editorManagerInterface) const {
		QStringList resultList;

		foreach (Id const &editor, editorManagerInterface->editors()) {
			foreach (Id const &diagram, editorManagerInterface->diagrams(editor)) {
				foreach (Id const &element, editorManagerInterface->elements(diagram)) {
					foreach (QString const &property, editorManagerInterface->propertyNames(element)) {
						QString const additionalString = ConvertingMethods::transformateOutput(callMethod(editorManagerInterface, editor, diagram, element, property), Id::rootId(), property);
						resultList.append(additionalString);
						resultList.append("|");
					}
				}
			}
		}
		return resultList;
	}
};

class MethodsTesterForQrxcAndInterpreter::EditorsListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForEditors
{
	virtual QString methodName() const {
		return "Editors";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(diagramId);
		Q_UNUSED(elementId);
		Q_UNUSED(propertyName);
		Q_UNUSED(editorManagerInterface);
		return ConvertingMethods::convertIdIntoStringList(editorId);
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new EditorsListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::DiagramsListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForEditors
{
	virtual QString methodName() const {
		return "Diagrams";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(diagramId);
		Q_UNUSED(elementId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertIdListIntoStringList(editorManagerInterface->diagrams(editorId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new DiagramsListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::ElementsListGeneratorWithIdParameter : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForDiagrams
{
	virtual QString methodName() const {
		return "Elements(Id const &diagram)";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(editorId);
		Q_UNUSED(elementId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertIdListIntoStringList(editorManagerInterface->elements(diagramId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new ElementsListGeneratorWithIdParameter();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::ElementsListGeneratorWithQStringParameters : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForDiagrams
{
	virtual QString methodName() const {
		return "Elements(QString const &editor, QString const &diagram)";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(elementId);
		Q_UNUSED(propertyName);
		QString const &editorName = editorId.editor();
		QString const &diagramName = diagramId.diagram();
		return editorManagerInterface->elements(editorName, diagramName);
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new ElementsListGeneratorWithQStringParameters();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::MouseGesturesListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Mouse gestures";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->mouseGesture(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new MouseGesturesListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::FriendlyNameListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Friendly name";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->friendlyName(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new FriendlyNameListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::DescriptionListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Description";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->description(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new DescriptionListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::PropertyDescriptionListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForProperties
{
	virtual QString methodName() const {
		return "Property description";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		QStringList result;
		try {
			result = ConvertingMethods::convertStringIntoStringList(editorManagerInterface->propertyDescription(elementId, propertyName));
		} catch (...) {
			result.append("method failed");
		}
		return result;
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new PropertyDescriptionListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::PropertyDisplayedNameListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForProperties
{
	virtual QString methodName() const {
		return "Property displayed name";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->propertyDisplayedName(elementId,propertyName));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new PropertyDisplayedNameListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::ContainedTypesListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Contained types";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertIdListIntoStringList(editorManagerInterface->containedTypes(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new ContainedTypesListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::ConnectedTypesListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Connected types";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const
	{
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertIdListIntoStringList(editorManagerInterface->connectedTypes(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new ConnectedTypesListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::UsedTypesListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Used types";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertIdListIntoStringList(editorManagerInterface->usedTypes(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new UsedTypesListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::EnumValuesListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Enum values";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		QString const &name = elementId.element();
		return (editorManagerInterface->enumValues(elementId, name));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new EnumValuesListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::TypeNameListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Type name";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		QString const &name = elementId.element();
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->typeName(elementId, name));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new TypeNameListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::AllChildrenTypesOfListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "All children types of";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return (editorManagerInterface->allChildrenTypesOf(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new AllChildrenTypesOfListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::IsEditorListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForEditors
{
	virtual QString methodName() const {
		return "Is editor (for editors)";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(elementId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertBoolIntoStringList(editorManagerInterface->isEditor(editorId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new IsEditorListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::IsDiagramListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForDiagrams
{
	virtual QString methodName() const {
		return "Is diagram (for diagrams)";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(elementId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertBoolIntoStringList(editorManagerInterface->isDiagram(diagramId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new IsDiagramListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::IsElementListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Is element (for elements)";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertBoolIntoStringList(editorManagerInterface->isElement(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new IsElementListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::PropertyNamesListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Property names";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return (editorManagerInterface->propertyNames(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new PropertyNamesListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::DefaultPropertyValuesListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForProperties
{
	virtual QString methodName() const {
		return "Property default value";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->defaultPropertyValue(elementId, propertyName));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new DefaultPropertyValuesListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::PropertiesWithDefaultValuesListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Properties with default values";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return (editorManagerInterface->propertiesWithDefaultValues(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new PropertiesWithDefaultValuesListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::HasElementListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Has element";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertBoolIntoStringList(editorManagerInterface->hasElement(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new HasElementListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::FindElementByTypeListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Find element by type";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		QString const &type = elementId.element();
		return ConvertingMethods::convertIdIntoStringList(editorManagerInterface->findElementByType(type));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new FindElementByTypeListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::IsGraphicalElementListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Is graphical element";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertBoolIntoStringList(editorManagerInterface->isGraphicalElementNode(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new IsGraphicalElementListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::IsNodeOrEdgeListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Is node or edge";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		QString const &editorName = editorId.editor();
		QString const &elementName = elementId.element();
		return ConvertingMethods::convertIntIntoStringList(editorManagerInterface->isNodeOrEdge(editorName, elementName));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new IsNodeOrEdgeListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::DiagramNameListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForDiagrams
{
	virtual QString methodName() const {
		return "Diagram name";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(elementId);
		Q_UNUSED(propertyName);
		QString const &editorName = editorId.editor();
		QString const &diagramName = diagramId.diagram();
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->diagramName(editorName, diagramName));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new DiagramNameListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::DiagramNodeNameListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForDiagrams
{
	virtual QString methodName() const {
		return "Diagram node name";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(elementId);
		Q_UNUSED(propertyName);
		QString const &editorName = editorId.editor();
		QString const &diagramName = diagramId.diagram();
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->diagramNodeName(editorName, diagramName));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new DiagramNodeNameListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::IsParentPropertyListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForProperties
{
	virtual QString methodName() const {
		return "Is parent property";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		return ConvertingMethods::convertBoolIntoStringList(editorManagerInterface->isParentProperty(elementId, propertyName));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new IsParentPropertyListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::ChildrenListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Children";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertIdListIntoStringList(editorManagerInterface->children(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new ChildrenListGenerator();
		return clonedGenerator;
	}
};

class MethodsTesterForQrxcAndInterpreter::ShapeListGenerator : public MethodsTesterForQrxcAndInterpreter::StringGeneratorForElements
{
	virtual QString methodName() const {
		return "Shape";
	}

	virtual QStringList callMethod(EditorManagerInterface *editorManagerInterface
			, Id const &editorId
			, Id const &diagramId
			, Id const &elementId
			, QString const &propertyName) const {
		Q_UNUSED(editorId);
		Q_UNUSED(diagramId);
		Q_UNUSED(propertyName);
		return ConvertingMethods::convertStringIntoStringList(editorManagerInterface->shape(elementId));
	}

	virtual UnifiedStringGenerator* clone() const {
		UnifiedStringGenerator* clonedGenerator = new ShapeListGenerator();
		return clonedGenerator;
	}
};

void MethodsTesterForQrxcAndInterpreter::testMethods()
{
	fillMethodsToTestList(pathToQrxcInterpreterMethodsToTest);

	mGeneratedList.append(testMethodIfExistsInList(EditorsListGenerator(), "editors"));
	mGeneratedList.append(testMethodIfExistsInList(DiagramsListGenerator(), "diagrams"));
	mGeneratedList.append(testMethodIfExistsInList(ElementsListGeneratorWithIdParameter(), "elements(Id)"));
	mGeneratedList.append(testMethodIfExistsInList(ElementsListGeneratorWithQStringParameters()
			, "elements(QString, QString)"));

	mGeneratedList.append(testMethodIfExistsInList(MouseGesturesListGenerator(), "mouseGestures"));
	mGeneratedList.append(testMethodIfExistsInList(FriendlyNameListGenerator(), "friendlyName"));
	mGeneratedList.append(testMethodIfExistsInList(DescriptionListGenerator(), "description"));
	mGeneratedList.append(testMethodIfExistsInList(PropertyDescriptionListGenerator(), "propertyDescription"));
	mGeneratedList.append(testMethodIfExistsInList(PropertyDisplayedNameListGenerator(), "propertyDisplayedName"));

	mGeneratedList.append(testMethodIfExistsInList(ContainedTypesListGenerator(), "containedTypes"));
	mGeneratedList.append(testMethodIfExistsInList(ConnectedTypesListGenerator(), "connectedTypes"));
	mGeneratedList.append(testMethodIfExistsInList(UsedTypesListGenerator(), "usedTypes"));
	mGeneratedList.append(testMethodIfExistsInList(EnumValuesListGenerator(), "enumValues"));
	mGeneratedList.append(testMethodIfExistsInList(TypeNameListGenerator(), "typeName"));
	mGeneratedList.append(testMethodIfExistsInList(AllChildrenTypesOfListGenerator(), "allChildrenTypesOf"));

	mGeneratedList.append(testMethodIfExistsInList(IsEditorListGenerator(), "isEditor"));
	mGeneratedList.append(testMethodIfExistsInList(IsDiagramListGenerator(), "isDiagram"));
	mGeneratedList.append(testMethodIfExistsInList(IsElementListGenerator(), "isElement"));

	mGeneratedList.append(testMethodIfExistsInList(PropertyNamesListGenerator(), "propertyNames"));
	mGeneratedList.append(testMethodIfExistsInList(DefaultPropertyValuesListGenerator(), "defaultPropertyValues"));
	mGeneratedList.append(testMethodIfExistsInList(PropertiesWithDefaultValuesListGenerator()
			, "propertiesWithDefaultValues"));

	mGeneratedList.append(testMethodIfExistsInList(HasElementListGenerator(), "hasElement"));
	mGeneratedList.append(testMethodIfExistsInList(FindElementByTypeListGenerator(), "findElementByType"));
	mGeneratedList.append(testMethodIfExistsInList(IsGraphicalElementListGenerator(), "isGraphicalElement"));

	mGeneratedList.append(testMethodIfExistsInList(IsNodeOrEdgeListGenerator(), "isNodeOrEdge"));
	mGeneratedList.append(testMethodIfExistsInList(DiagramNameListGenerator(), "diagramName"));
	mGeneratedList.append(testMethodIfExistsInList(DiagramNodeNameListGenerator(), "diagramNodeName"));
	mGeneratedList.append(testMethodIfExistsInList(IsParentPropertyListGenerator(), "isParentProperty"));
	mGeneratedList.append(testMethodIfExistsInList(ChildrenListGenerator(), "children"));
	mGeneratedList.append(testMethodIfExistsInList(ShapeListGenerator(), "shape"));
}


QList<QPair<QString, QPair<QString, QString> > > MethodsTesterForQrxcAndInterpreter::generatedResult()
{
	testMethods();
	return mGeneratedList;
}

UnifiedStringGenerator * MethodsTesterForQrxcAndInterpreter::initGeneratorWithFirstInterface(UnifiedStringGenerator const &generator) const
{
	StringGenerator *clonedGenerator = dynamic_cast<StringGenerator *>(generator.clone());

	clonedGenerator->init(mQrxcGeneratedPlugin);
	return clonedGenerator;
}

UnifiedStringGenerator * MethodsTesterForQrxcAndInterpreter::initGeneratorWithSecondInterface(UnifiedStringGenerator const &generator) const
{
	StringGenerator *clonedGenerator = dynamic_cast<StringGenerator *>(generator.clone());

	clonedGenerator->init(mInterpreterGeneratedPlugin);
	return clonedGenerator;
}
