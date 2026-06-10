/****************************************************************************
** Meta object code from reading C++ file 'TrayApp.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/TrayApp.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TrayApp.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSTrayAppENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSTrayAppENDCLASS = QtMocHelpers::stringData(
    "TrayApp",
    "onAddVirtualDisplay",
    "",
    "onTrayIconActivated",
    "QSystemTrayIcon::ActivationReason",
    "reason",
    "onAddNDISender",
    "DisplayInfo*",
    "info",
    "onRemoveNDISender",
    "onRemoveVirtualDisplay",
    "onOpenSettings"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSTrayAppENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   50,    2, 0x08,    1 /* Private */,
       3,    1,   51,    2, 0x08,    2 /* Private */,
       6,    1,   54,    2, 0x08,    4 /* Private */,
       9,    1,   57,    2, 0x08,    6 /* Private */,
      10,    1,   60,    2, 0x08,    8 /* Private */,
      11,    0,   63,    2, 0x08,   10 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject TrayApp::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSTrayAppENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSTrayAppENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSTrayAppENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TrayApp, std::true_type>,
        // method 'onAddVirtualDisplay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTrayIconActivated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSystemTrayIcon::ActivationReason, std::false_type>,
        // method 'onAddNDISender'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<DisplayInfo *, std::false_type>,
        // method 'onRemoveNDISender'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<DisplayInfo *, std::false_type>,
        // method 'onRemoveVirtualDisplay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<DisplayInfo *, std::false_type>,
        // method 'onOpenSettings'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void TrayApp::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TrayApp *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onAddVirtualDisplay(); break;
        case 1: _t->onTrayIconActivated((*reinterpret_cast< std::add_pointer_t<QSystemTrayIcon::ActivationReason>>(_a[1]))); break;
        case 2: _t->onAddNDISender((*reinterpret_cast< std::add_pointer_t<DisplayInfo*>>(_a[1]))); break;
        case 3: _t->onRemoveNDISender((*reinterpret_cast< std::add_pointer_t<DisplayInfo*>>(_a[1]))); break;
        case 4: _t->onRemoveVirtualDisplay((*reinterpret_cast< std::add_pointer_t<DisplayInfo*>>(_a[1]))); break;
        case 5: _t->onOpenSettings(); break;
        default: ;
        }
    }
}

const QMetaObject *TrayApp::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TrayApp::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSTrayAppENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TrayApp::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
