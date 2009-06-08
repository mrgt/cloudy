#ifndef CLOUDY_EDITOR_HPP
#define CLOUDY_EDITOR_HPP

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <iostream>

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
	       std::cerr << "add_double " << name << "\n";
	       QObject *edit = new Editable_double(ref);
	       _editables.push_back(edit);

	       QLabel *label = new QLabel();
	       label->setText(name.c_str());

	       //QDoubleSpinBox *box = new QDoubleSpinBox();
	       DoubleSlider *box = new DoubleSlider;
	       box->setRange(min,max);
	       //box->setSingleStep((max-min)/1000.0);
	       box->setValue(ref);

	       connect(box, SIGNAL(valueChanged(double)),
	               edit, SLOT(set_value(double)));
	       connect(box, SIGNAL(valueChanged(double)),
	               this, SLOT(stateChangedSlot()));

	       _layout->addWidget(label, _current_row, 0);
	       _layout->addWidget(box, _current_row, 1);
	       _current_row++;
	    }

	    void add_bool(const std::string &name, bool &ref)
	    {
	       std::cerr << "add_double " << name << "\n";
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
