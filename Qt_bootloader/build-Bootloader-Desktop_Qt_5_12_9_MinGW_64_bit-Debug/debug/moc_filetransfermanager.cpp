/****************************************************************************
** Meta object code from reading C++ file 'filetransfermanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.9)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Qt_Bootloader/filetransfermanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'filetransfermanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.9. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FileTransferManager_t {
    QByteArrayData data[16];
    char stringdata0[186];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FileTransferManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FileTransferManager_t qt_meta_stringdata_FileTransferManager = {
    {
QT_MOC_LITERAL(0, 0, 19), // "FileTransferManager"
QT_MOC_LITERAL(1, 20, 15), // "transferStarted"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 8), // "fileName"
QT_MOC_LITERAL(4, 46, 8), // "uint32_t"
QT_MOC_LITERAL(5, 55, 8), // "fileSize"
QT_MOC_LITERAL(6, 64, 16), // "transferProgress"
QT_MOC_LITERAL(7, 81, 9), // "sentBytes"
QT_MOC_LITERAL(8, 91, 10), // "totalBytes"
QT_MOC_LITERAL(9, 102, 16), // "transferFinished"
QT_MOC_LITERAL(10, 119, 7), // "success"
QT_MOC_LITERAL(11, 127, 7), // "message"
QT_MOC_LITERAL(12, 135, 5), // "error"
QT_MOC_LITERAL(13, 141, 8), // "errorMsg"
QT_MOC_LITERAL(14, 150, 17), // "onSocketReadyRead"
QT_MOC_LITERAL(15, 168, 17) // "onTransferTimeout"

    },
    "FileTransferManager\0transferStarted\0"
    "\0fileName\0uint32_t\0fileSize\0"
    "transferProgress\0sentBytes\0totalBytes\0"
    "transferFinished\0success\0message\0error\0"
    "errorMsg\0onSocketReadyRead\0onTransferTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FileTransferManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x06 /* Public */,
       6,    2,   49,    2, 0x06 /* Public */,
       9,    2,   54,    2, 0x06 /* Public */,
      12,    1,   59,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      14,    0,   62,    2, 0x08 /* Private */,
      15,    0,   63,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 4,    3,    5,
    QMetaType::Void, 0x80000000 | 4, 0x80000000 | 4,    7,    8,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   10,   11,
    QMetaType::Void, QMetaType::QString,   13,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void FileTransferManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FileTransferManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->transferStarted((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< uint32_t(*)>(_a[2]))); break;
        case 1: _t->transferProgress((*reinterpret_cast< uint32_t(*)>(_a[1])),(*reinterpret_cast< uint32_t(*)>(_a[2]))); break;
        case 2: _t->transferFinished((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: _t->error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->onSocketReadyRead(); break;
        case 5: _t->onTransferTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FileTransferManager::*)(const QString & , uint32_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileTransferManager::transferStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FileTransferManager::*)(uint32_t , uint32_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileTransferManager::transferProgress)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (FileTransferManager::*)(bool , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileTransferManager::transferFinished)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (FileTransferManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FileTransferManager::error)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FileTransferManager::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_FileTransferManager.data,
    qt_meta_data_FileTransferManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FileTransferManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FileTransferManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FileTransferManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int FileTransferManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void FileTransferManager::transferStarted(const QString & _t1, uint32_t _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FileTransferManager::transferProgress(uint32_t _t1, uint32_t _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void FileTransferManager::transferFinished(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void FileTransferManager::error(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
