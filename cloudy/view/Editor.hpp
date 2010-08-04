#ifndef CLOUDY_EDITOR_HPP
#define CLOUDY_EDITOR_HPP

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include <QColorDialog>
#include <iostream>
#include <math.h>
#include <cloudy/mesh/Gradient.hpp>

namespace cloudy
{
   namespace view
   {
      class DoubleSlider: public QWidget
      {
	    Q_OBJECT;

	    double _min, _max;
	    QLabel *_left, *_right;
	    QSlider *_slider;
	    QHBoxLayout *_layout;

	 signals:
	    void valueChanged(double);

	 public slots:
	    void sliderValueChanged(int value)
	    {
	       double d = double(value)/double(_slider->tickInterval());
	       emit valueChanged(_min + d * (_max - _min));
	    }

	 public:
	    DoubleSlider(QWidget *parent=0): QWidget(parent)
	    {
	       _slider = new QSlider(Qt::Horizontal);
	       _left = new QLabel;
	       _right = new QLabel;
	       _layout = new QHBoxLayout;

	       _layout->addWidget(_left);
	       _layout->addWidget(_slider);
	       _layout->addWidget(_right);
	       setLayout(_layout);

	       setRange(0.0, 1.0);
	       setResolution(100);

	       connect(_slider, SIGNAL(valueChanged(int)),
	               this, SLOT(sliderValueChanged(int)));
	    }

	    void setRange(double min, double max)
	    {
	       _min = min;
	       _max = max;
	       _left->setNum(_min);
	       _right->setNum(_max);
	    }

	    void setValue(double v)
	    {
	       double t = (v - _min)/(_max - _min);
	       _slider->setValue(_slider->tickInterval()*t);
	    }

	    void setResolution(size_t N)
	    {
	       _slider->setTickInterval(N);
	    }
      };

      class Editable_double: public QObject
      {
	    Q_OBJECT;
	    double &_value;

	 public:
	    Editable_double(double &value):
	       _value(value)
	    {}

	 public slots:
	    void set_value(double v)
	    {
	       _value = v;
	    }

	    void set_value(const QString&s)
	    {
	      bool ok;
	      double d = s.toDouble(&ok);

	      if (ok)
		{
		  _value = d;
		}
	    }
      };


      class Editable_integer: public QObject
      {
	    Q_OBJECT;
	    int &_value;

	 public:
	    Editable_integer(int &value):
	       _value(value)
	    {}

	 public slots:
	    void set_value(int v)
	    {
	       _value = v;
	    }
      };

      inline QColor color_to_qcolor(const Color &c)
      {
	 return QColor(int(c._r*255.0),
	               int(c._g*255.0),
	               int(c._b*255.0));
      }

      inline Color qcolor_to_color(const QColor &c)
      {
	 return Color(c.redF(), c.greenF(), c.blueF());
      }

      class Editable_color: public QObject
      {
	    Q_OBJECT;
	    Color &_value;

	 public:
	    Editable_color(Color &value):
	       _value(value)
	    {}

	 public slots:
	    void set_value(const QColor & v)
	    {
	       *(&_value) = qcolor_to_color(v);
	    }

	    void select_color()
	    {
	       set_value(QColorDialog::getColor(color_to_qcolor(_value)));
	    } 
      };


      class Editable_bool: public QObject
      {
	    Q_OBJECT;
	    bool &_value;

	 public:
	    Editable_bool(bool &value):
	       _value(value)
	    {}

	 public slots:
	    void state_changed(int v)
	    {
	       _value = (v != 0);
	    }
      };

      class Editor : public QWidget
      {
	    Q_OBJECT;
	    std::vector<QObject*> _editables;
	    QGridLayout *_layout;
	    size_t _current_row;

	 public:
	    Editor()
	    {
	       _layout = new QGridLayout;
	       _current_row = 0;
	       setLayout(_layout);
	    }

	    ~Editor()
	    {
	       foreach(QObject *obj, _editables)
		  delete obj;
	    }
	    
	    void add_double(const std::string &name, double &ref,
	                    double min = 0.0, double max = 1.0)
	    {
	       QObject *edit = new Editable_double(ref);
	       _editables.push_back(edit);

	       QLabel *label = new QLabel();
	       label->setText(name.c_str());

#if 1
	       QLineEdit *box = new QLineEdit;
	       box->setText(QString::number(ref));

	       connect(box, SIGNAL(textChanged(const QString&)),
	               edit, SLOT(set_value(const QString&)));
	       connect(box, SIGNAL(textChanged(const QString&)),
	               this, SLOT(stateChangedSlot()));
#else
	       //QDoubleSpinBox *box = new QDoubleSpinBox();
	       DoubleSlider *box = new DoubleSlider;
	       box->setRange(min,max);
	       //box->setSingleStep((max-min)/1000.0);
	       box->setValue(ref);

	       connect(box, SIGNAL(valueChanged(double)),
	               edit, SLOT(set_value(double)));
	       connect(box, SIGNAL(valueChanged(double)),
	               this, SLOT(stateChangedSlot()));
#endif

	       _layout->addWidget(label, _current_row, 0);
	       _layout->addWidget(box, _current_row, 1);
	       _current_row++;
	    }

	    void add_double_spin(const std::string &name, double &ref,
				 double min = 0.0, double max = 1.0,
				 int prec=3)
	    {
	       QObject *edit = new Editable_double(ref);
	       _editables.push_back(edit);

	       QLabel *label = new QLabel();
	       label->setText(name.c_str());

	       QDoubleSpinBox *box = new QDoubleSpinBox();

	       box->setValue(ref);
	       box->setDecimals(prec);	      
	       box->setRange(min,max);
	       box->setSingleStep((max-min)/pow(10.0,prec));

	       connect(box, SIGNAL(valueChanged(double)),
	               edit, SLOT(set_value(double)));
	       connect(box, SIGNAL(valueChanged(double)),
	               this, SLOT(stateChangedSlot()));

	       _layout->addWidget(label, _current_row, 0);
	       _layout->addWidget(box, _current_row, 1);
	       _current_row++;
	    }


	    void add_integer_spin(const std::string &name, int &ref,
				  int min, int max)
	    {
	       QObject *edit = new Editable_integer(ref);
	       _editables.push_back(edit);

	       QLabel *label = new QLabel();
	       label->setText(name.c_str());

	       QSpinBox *box = new QSpinBox();
	       box->setRange(min,max);
	       box->setSingleStep(1);
	       box->setValue(ref);

	       connect(box, SIGNAL(valueChanged(int)),
	               edit, SLOT(set_value(int)));
	       connect(box, SIGNAL(valueChanged(int)),
	               this, SLOT(stateChangedSlot()));

	       _layout->addWidget(label, _current_row, 0);
	       _layout->addWidget(box, _current_row, 1);
	       _current_row++;
	    }

	    void add_bool(const std::string &name, bool &ref)
	    {
	       QObject *edit = new Editable_bool(ref);
	       _editables.push_back(edit);

	       QLabel *label = new QLabel();
	       label->setText(name.c_str());

	       QCheckBox *box = new QCheckBox();
	       box->setCheckState(ref ? Qt::Checked : Qt::Unchecked);
	       connect(box, SIGNAL(stateChanged(int)),
	               edit, SLOT(state_changed(int)));

	       connect(box, SIGNAL(stateChanged(int)),
	               this, SLOT(stateChangedSlot()));

	       _layout->addWidget(label, _current_row, 0);
	       _layout->addWidget(box, _current_row, 1);
	       _current_row++;
	    }

	    void add_color(const std::string &name, Color &ref)
	    {
	       QObject *edit = new Editable_color(ref);
	       _editables.push_back(edit);

	       QLabel *label = new QLabel();
	       label->setText(name.c_str());

	       QPushButton *box = new QPushButton();
	       connect(box, SIGNAL(clicked()),
	               edit, SLOT(select_color()));
	       _layout->addWidget(label, _current_row, 0);
	       _layout->addWidget(box, _current_row, 1);
	       _current_row++;
	    }


	    void finish()
	    {
	       _layout->setRowStretch(_current_row+1,2);
	    }

	 signals:
	    void stateChanged();
	    
	 public slots:
	    void stateChangedSlot()
	    {
	       emit stateChanged();
	    }
	    
      };
   }
}

#endif
