/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PROGRAMOPTIONSTYPE_H
#define PROGRAMOPTIONSTYPE_H

// #include <QObject>
#include <QVariant>
#include <QColor>
#include <sys/stat.h>
#include <QAction>
#include<QIcon>

class QSpinBox;
class QSlider;
class ProgramOptions;

/**
 * @ingroup HelperClasses
 * @brief Interface class for single program option
 *
 */
class Option : public QObject
{
	Q_OBJECT
	
	QString name;
	QString optClass;

	QString description;
	QString descriptionShort;
public:
	Option(const QString& name, const QString& optClass);
	~Option() override {}
	virtual void resetValue() = 0;
	const QString& getName()                                  const { return name; }
	const QString& getClass()                                 const { return optClass; }
	
	virtual QVariant getVariant() = 0;
	virtual void setVariant(const QVariant& variant) = 0;

	const QString& getDescription()                           const { return description; }
	const QString& getDescriptionShort()                      const { return descriptionShort; }

	virtual void setDescriptions(const QString& shortDesc, const QString& longDesc)
	                                                                { description = longDesc; descriptionShort = shortDesc; }

	virtual bool isDefault() const = 0;
};



/**
 * @ingroup HelperClasses
 * @brief A program option that hold a boolean
 *
 */
class OptionBool : public Option
{
	Q_OBJECT
	
	friend ProgramOptions;
	
	bool value;
	bool defaultValue;

	QAction* action;

	OptionBool(const bool v, const QString& name, const QString& optClass) : Option(name, optClass), value(v), defaultValue(v), action(new QAction(this))
	{
		action->setText(QObject::tr(name.toStdString().c_str()));
		action->setCheckable(true);
		action->setChecked(v);
		connect(action, &QAction::triggered      , this   , &OptionBool::setValue);
// 		connect(this   , &OptionBool::valueChanged, &action, &QAction::setChecked );
	}
	OptionBool(const OptionBool&) = delete;
	OptionBool& operator=(const OptionBool&) = delete;
	
	void setValuePrivat(bool v)
	{
		if(v!=value)
		{
			value = v;
			action->setChecked(v);
			emit(valueChanged(v));
			emit(valueChangedInvers(!v));
			if(v)
				emit(trueSignal());
			else
				emit(falseSignal());
		}
	}
	
public:
	void resetValue()                              override { setValuePrivat(defaultValue); };

	
	bool getValue()   const                                         { return value; }
	bool operator()() const                                         { return value; }

	QAction* getAction()                                            { return action; }
	
	QVariant getVariant()                          override { return QVariant(value); }
	void setVariant(const QVariant& variant)       override { setValuePrivat(variant.toBool()); }

	bool isDefault() const                         override { return value == defaultValue; }

	void setDescriptions(const QString& shortDesc, const QString& longDesc) override;
public slots:
	void setValue(bool v)                                           { setValuePrivat(v); }
	void setValueInvers(bool v)                                     { setValuePrivat(!v); }
	
signals:
	void valueChanged(bool v);
	void valueChangedInvers(bool v);
	void trueSignal();
	void falseSignal();
};

/**
 * @ingroup HelperClasses
 * @brief A program option that hold a integer
 *
 */
class OptionInt : public Option
{
	Q_OBJECT

	friend ProgramOptions;

	int value;
	int defaultValue;

	int valueMin;
	int valueMax;
	int valueStepSize;

	QAction* inputDialogAction;

	OptionInt(const int v, const QString& name, const QString& optClass, int min = 0, int max = 100, int stepSize = 1)
	: Option(name, optClass)
	, value(v)
	, defaultValue(v)
	, valueMin(min)
	, valueMax(max)
	, valueStepSize(stepSize)
	, inputDialogAction(new QAction(this))
	{
		connect(inputDialogAction, &QAction::triggered, this, &OptionInt::showInputDialog);
	}
	OptionInt(const OptionInt&) = delete;
	OptionInt& operator=(const OptionInt&) = delete;
public:
	void resetValue()                              override { value = defaultValue; emit(valueChanged(value)); };

	int getValue()   const                                          { return value; }
	int operator()() const                                          { return value; }

	QAction* getInputDialogAction()                                 { return inputDialogAction; }
	QSpinBox* createSpinBox(QWidget* parent);
	
	QVariant getVariant()                          override { return QVariant(value); }
	void setVariant(const QVariant& variant)       override { value = variant.toInt(); }

	void setDescriptions(const QString& shortDesc, const QString& longDesc) override;

	bool isDefault() const                         override { return value == defaultValue; }
public slots:
	void setValue(int v)                                            { if(value!=v) { value = v; emit(valueChanged(v));} }
	void showInputDialog();

signals:
	void valueChanged(int v);
};


/**
 * @ingroup HelperClasses
 * @brief A program option that hold a double value
 *
 */
class OptionDouble : public Option
{
	Q_OBJECT

	friend ProgramOptions;

	double value;
	double defaultValue;

	double valueMin;
	double valueMax;
	double valueStepSize;

	QAction* inputDialogAction;

	OptionDouble(const double v, const QString& name, const QString& optClass, double min = 0, double max = 100, double stepSize = 1);
	OptionDouble(const OptionDouble&) = delete;
	OptionDouble& operator=(const OptionDouble&) = delete;
public:
	void resetValue()                              override { value = defaultValue; emit(valueChanged(value)); };

	double getValue()   const                                       { return value; }
	double operator()() const                                       { return value; }

	QAction* getInputDialogAction()                                 { return inputDialogAction; }
	QSlider* createSlider(Qt::Orientation orientation, QWidget* parent);

	QVariant getVariant()                          override { return QVariant(value); }
	void setVariant(const QVariant& variant)       override { value = variant.toDouble(); }

	void setDescriptions(const QString& shortDesc, const QString& longDesc) override;

	bool isDefault() const                         override { return value == defaultValue; }
public slots:
	void setValue(double v)                                         { if(value!=v) { value = v; emit(valueChanged(v));} }
	void showInputDialog();

signals:
	void valueChanged(double v);
};

/**
 * @ingroup HelperClasses
 * @brief A program option that hold a string
 *
 */
class OptionString : public Option
{
	Q_OBJECT

	friend ProgramOptions;

	QString value;
	QString defaultValue;

	OptionString(const QString& v, const QString& name, const QString& optClass) : Option(name, optClass), value(v), defaultValue(v) {}
	OptionString(const OptionString&) = delete;
	OptionString& operator=(const OptionString&) = delete;
public:
	void resetValue()                              override { value = defaultValue; emit(valueChanged(value)); };

	const QString& getValue()   const                               { return value; }
	const QString& operator()() const                               { return value; }
	
	QVariant getVariant()                          override { return QVariant(value); }
	void setVariant(const QVariant& variant)       override { value = variant.toString(); }

	bool isDefault() const                         override { return value == defaultValue; }
public slots:
	void setValue(const QString& v)                                 { value = v; emit(valueChanged(v)); }

signals:
	void valueChanged(const QString& v);
};


/**
 * @ingroup HelperClasses
 * @brief A program option that hold a QColor
 *
 */
class OptionColor : public Option
{
	Q_OBJECT

	friend ProgramOptions;

	QColor value;
	QColor defaultValue;

	QAction* colorDialogAction;

	OptionColor(const QColor& v, const QString& name, const QString& optClass) : Option(name, optClass), value(v), defaultValue(v), colorDialogAction(new QAction(this))
	{
// 		colorDialogAction->setIcon(QIcon(":/icons/color_wheel.png"));
		connect(colorDialogAction, &QAction::triggered, this, &OptionColor::showColorDialog);
	}
	OptionColor(const OptionColor&) = delete;
	OptionColor& operator=(const OptionColor&) = delete;
public:
	void resetValue()                              override { value = defaultValue; emit(valueChanged(value)); };

	const QColor& getValue()   const                                { return value; }
	const QColor& operator()() const                                { return value; }

	QAction* getColorDialogAction()                                 {return colorDialogAction; }
	
	QVariant getVariant()                          override { return QVariant(value); }
	void setVariant(const QVariant& variant)       override { value = variant.value<QColor>(); }

	void setDescriptions(const QString& shortDesc, const QString& longDesc) override;

	bool isDefault() const                         override { return value == defaultValue; }
public slots:
	void setValue(const QColor& v)                                  { value = v; emit(valueChanged(v)); }
	void showColorDialog();

signals:
	void valueChanged(const QColor& v);
};



#endif


