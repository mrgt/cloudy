#ifndef CLOUDY_EDITOR_HPP
#define CLOUDY_EDITOR_HPP

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <iostream>

namespace cloudy
{
   namespace view
   {
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
	    
	    void add_double(const std::string &name, double &ref)
	    {
	       std::cerr << "add_double " << name << "\n";
	       QObject *edit = new Editable_double(ref);
	       _editables.push_back(edit);

	       QLabel *label = new QLabel();
	       label->setText(name.c_str());

	       QDoubleSpinBox *box = new QDoubleSpinBox();
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
