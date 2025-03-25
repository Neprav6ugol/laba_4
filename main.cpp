#include <QApplication>
#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <vector>
#include <QDebug>
#include <Set>
#include "./ui_mainwindow.h"
#include <QSizeGrip>
#include <QSplitter>
#include <QGraphicsItem>
#include <QPushButton>
#include <QAbstractGraphicsShapeItem>
#include <QPainterPath>
#include <QGraphicsItem>
#include <QColorDialog>

using namespace std;

void debug(std::string info) {
    qDebug() << info.c_str() << Qt::endl;
}
void debug2(QString info) {
    qDebug() << info << Qt::endl;
}


class IShape {
public:

    virtual ~IShape() {}  // почему?

    virtual QRectF getRect() = 0;
    virtual void changeSize(QRectF) = 0;
    virtual QPointF getMapFromScene(QPointF) = 0;
    virtual bool isContains(QPointF p) = 0;

    virtual void setNewPos(double w, double h) = 0;
    virtual void addOnScene(QGraphicsScene *scene) = 0;

    virtual void showSelected() = 0;

    virtual void showUnSelected(QColor color) = 0;
    virtual void setColor(QColor color) = 0;
    QColor color;


};

class CustomShape : protected QAbstractGraphicsShapeItem, public IShape {
protected:
    int size;
public:
    QGraphicsScene *scene;
    QColor color;

    // IShape *item = nullptr;
    CustomShape(int posX, int posY, int size, QGraphicsItem *parent = nullptr)
        : QAbstractGraphicsShapeItem(parent) {
        setAcceptHoverEvents(true);
        this->size = size;
        this->movable = false;


    }

    void removeFromScene() {
        this->scene->removeItem(dynamic_cast<QGraphicsItem*>(this));
    }

    QRectF getRect() override {
        return boundingRect();
    }

    virtual void changeSize(QRectF rect) override {
        size = rect.height();
    }

    QPointF getMapFromScene(QPointF pos) {
        return this->mapFromScene(pos);
    }
    bool isContains(QPointF p) {
        return this->contains(p);
    }

    void setNewPos(double x, double y) {
        prepareGeometryChange();
        this->setPos(x, y);
    }
    void addOnScene(QGraphicsScene *scene) {
        scene->addItem(this);
        this->scene = scene;
    }

    virtual void showSelected()  {
        this->setPen(QPen(Qt::red));
    }

    virtual void showUnSelected(QColor color)  {
        this->setPen(QPen(color));
    }
    void setColor(QColor color) {
        this->color = color;
    }

    bool isIntersect( CustomShape *anotherItem)  {
        // auto me = dynamic_cast<QGraphicsItem*>(this);
        // auto you = dynamic_cast<QGraphicsItem*>(anotherItem);
        if (!anotherItem || !this)
            return false;

        QRectF r1 = this->sceneBoundingRect();
        QRectF r2 = anotherItem->sceneBoundingRect();
        return r1.intersects(r2);
    }

    bool isFocused(QPointF scenePos)  {
        QPointF localPos = this->getMapFromScene(scenePos);
        return this->isContains(localPos);
    }

    void scale(double mlt, QGraphicsView *view) {
        QAbstractGraphicsShapeItem *item2 = dynamic_cast<QAbstractGraphicsShapeItem*>(this);
        if (!item2) return;


        //QRectF curRect = item2->mapRectToScene(item2->boundingRect());
        QRectF curRect = item2->boundingRect().translated(item2->pos());
        QRectF newRect = this->getRect();
        QRectF tmp = curRect;
        newRect.setHeight(curRect.height() * mlt);
        newRect.setWidth(curRect.width() * mlt);
        curRect.setHeight(curRect.height() * mlt);
        curRect.setWidth(curRect.width() * mlt);
        QRectF viewRect = view->mapToScene(view->viewport()->rect()).boundingRect();
        if (viewRect.contains(curRect)) {
            this->changeSize(newRect);
        }

        prepareGeometryChange();
        this->update();
    }

    static bool isShapeRectInView(QRectF itemRect, QGraphicsView *view) {
        // перед созданием Shape во View проверяется что предпологаемая фигура не выходит за границы

        QRectF viewRect = view->mapToScene(view->viewport()->rect()).boundingRect();

        return viewRect.contains(itemRect);

    }
    bool isShapeRectInView(QGraphicsView *view) {
        // после создания Shape вызывается перед тем как изменить положение фигуры

        QRectF viewRect = view->mapToScene(view->viewport()->rect()).boundingRect();

        return viewRect.contains(this->sceneBoundingRect());

    }

    void move(double difX, double difY, QGraphicsView *view) {
        QRectF itemRect = this->mapRectToScene(this->boundingRect());
        QRectF newItemRect = itemRect.translated(difX, difY);

        QRectF viewRect = view->mapToScene(view->viewport()->rect()).boundingRect();

        if (viewRect.contains(newItemRect)) {
            this->setPos(this->pos() + QPointF(difX, difY));
        }
    }

    void setMovable(bool val) {
        this->movable = val;
    }

    void drawOnScene(QPointF position, QGraphicsView *view, QColor color) {
        this->color = color;

        this->setBrush(QBrush(color)); // Устанавливаем кисть
        this->setPen(QPen(color, 2));
        // Установите размер фигуры
        QRectF rect = this->getRect();
        double w = rect.width();
        double h = rect.height();
        this->setPos(position.x(), position.y());

        this->addOnScene(view->scene());
    }

    bool ismovable() {
        return this->movable;
    }



    void moveInViewBack(QGraphicsView *view) {
        QRectF itemRect = this->sceneBoundingRect();
        QRectF viewRect = view->mapToScene(view->viewport()->rect()).boundingRect();
        this->move(0.0d, -abs(viewRect.bottom() - itemRect.bottom()), view);
    }

private:
    bool movable;

};

class MyCircleItem : public CustomShape {
public:
    MyCircleItem(int posX, int posY, int size, QGraphicsItem *parent = nullptr)
        : CustomShape(posX, posY, size, parent) {
        setAcceptHoverEvents(true);
    }

    QRectF boundingRect() const override {
        return QRectF(-size / 2, -size / 2, size, size);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        painter->setBrush(this->brush());
        painter->setPen(this->pen());
        painter->drawEllipse(boundingRect());
    }
};

class MySquareItem : public CustomShape {
public:
    MySquareItem(int posX, int posY, int size, QGraphicsItem *parent = nullptr)
        : CustomShape(posX, posY, size, parent) {
        setAcceptHoverEvents(true);
    }

    QRectF boundingRect() const override {
        return QRectF(-size / 2, -size / 2, size, size);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        painter->setBrush(this->brush());
        painter->setPen(this->pen());
        painter->drawRect(boundingRect());
    }
};

// Класс для треугольников
class MyTriangleItem : public CustomShape {
public:
    MyTriangleItem(int posX, int posY, int size, QGraphicsItem *parent = nullptr)
        : CustomShape(posX, posY, size, parent) {
        setAcceptHoverEvents(true);
    }

    QRectF boundingRect() const override {
        return QRectF(-size/2, -size/2, size, size);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QPolygonF polygon;
        polygon << QPointF(-size/2, size/2) << QPointF(size/2, size/2) << QPointF(0, -size/2) << QPointF(-size/2, size/2);
        painter->setBrush(this->brush());
        painter->setPen(this->pen());
        painter->drawPolygon(polygon);
    }
};

// Класс для линий
class MyLineItem : public CustomShape {
public:
    MyLineItem(int posX, int posY, int size, QGraphicsItem *parent = nullptr)
        : CustomShape(posX, posY, size, parent) {
        setAcceptHoverEvents(true);
    }

    QRectF boundingRect() const override {
        // Границы должны учитывать ТЕКУЩУЮ ширину пера
        qreal currentPenW = pen().widthF(); // Берем из QAbstractGraphicsShapeItem
        // Запас для концов пера и антиалиасинга
        qreal padding = currentPenW / 2.0 + 2.0;
        return QRectF(-padding, -size / 2.0 - 1, 2 * padding, size + 2);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        painter->setBrush(this->brush());
        painter->setPen(this->pen());;

        painter->drawLine(0, -size/2, 0, size/2);
    }



    void showSelected()  {
        // this->setBrush(QBrush(Qt::red));
        this->setPen(QPen(Qt::red, 5));
    }

    void showUnSelected(QColor color)  {
        this->setPen(QPen(color, 5));
    }

};

class MyTrapezoidItem : public CustomShape {
public:
    MyTrapezoidItem(int posX, int posY, int size, int height, QGraphicsItem *parent = nullptr)
        : CustomShape(posX, posY, size, parent) {
        setAcceptHoverEvents(true);
        this->height = height; // Сохраняем высоту для рисования
        this->size = size;
    }

    void changeSize(QRectF rect) override {
        size = rect.width();
        height = rect.height();
    }

    QRectF boundingRect() const override {
        return QRectF(-size / 2, -height / 2, size, height);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        QPolygonF polygon;
        polygon << QPointF(-size / 2, height / 2) // Левая нижняя
                << QPointF(size / 2, height / 2)  // Правая нижняя
                << QPointF(size / 4, -height / 2) // Верхняя левая
                << QPointF(-size / 4, -height / 2); // Верхняя правая

        painter->setBrush(this->brush());
        painter->setPen(this->pen());
        painter->drawPolygon(polygon);
    }

private:
    int height; // Высота параллелограмма
};


class MyStorage {
private:
    QVector<CustomShape*> items;        // Все созданные элементы

public:
    // Конструктор
    MyStorage() {}

    // Деструктор
    ~MyStorage() {
        clearAll();
    }

    // Удаление всех элементов
    void clearAll() {
        for (CustomShape* item : items) {
            if (item) {
                delete item;      // освобождаем память
            }
        }
        items.clear();
    }

    // Удаление только выбранных
    void clearSelected() {
        for (auto it = items.begin(); it != items.end(); ) {
            if (*it && (*it)->ismovable()) {
                if ((*it)->scene) {
                    (*it)->removeFromScene(); // Удаляем со сцены
                }
                delete *it;  // Освобождаем память
                it = items.erase(it); // Удаляем элемент из вектора и получаем итератор на следующий
            } else {
                ++it; // Переходим к следующему элементу
            }
        }
    }

    // Добавить элемент в общий список элементов
    void addItem(CustomShape* item) {
        items.push_back(item);
    }


    // Снять выделение со всех элементов
    void unselectAll() {
        for (CustomShape* item : items) {
            if (!item) continue;
            item->setMovable(false);
            item->showUnSelected(item->color);
        }
    }

    // Получить элемент по индексу
    CustomShape* getItem(int index) {
        if (index >= 0 && index < items.size()) {
            return items[index];
        }
        return nullptr;
    }

    // Получить количество элементов
    int count() {
        return items.size();
    }

    // Получить выбранные элементы
    QVector<CustomShape*> getSelectedItems() {
        QVector<CustomShape*> selectedItems;
        for (auto i : items) {
            if (i && i->ismovable()) {
                selectedItems.append(i);
            }
        }
        return selectedItems;
    }
};

class MyView : public QGraphicsView {
public:
    QColor color = QColor(
        QRandomGenerator::global()->bounded(240),
        QRandomGenerator::global()->bounded(256),
        QRandomGenerator::global()->bounded(256)
        );

    // [!] Теперь при создании MyView мы сразу создаём внутренний storage
    MyStorage storage;  // хранит все наши объекты

    MyView(QGraphicsScene *scene, QWidget *parent = nullptr)
        : QGraphicsView(scene, parent)
    {
        setMouseTracking(true);
        //setBackgroundBrush(Qt::black);
        setScene(scene);
        // adjustViewSize();
        setRenderHint(QPainter::Antialiasing);
        //setDragMode(QGraphicsView::ScrollHandDrag);
        //setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //setMinimumSize(100, 100); // Установите минимальный размер,

    }

    void setItemMode(std::string mode) {
        this->itemMode = mode;
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QGraphicsView::resizeEvent(event);
        for (int i = 0; i < storage.count(); i++) {
            CustomShape *item = storage.getItem(i);
            if (!item) continue;
            if (!(item->isShapeRectInView(this))) {
                item->moveInViewBack(this);
            }
        }
    }
    void keyPressEvent(QKeyEvent *event) override {
        // Удаление выбраных элементов
        if (event->key() == Qt::Key_Delete) {
            storage.clearSelected();
        }
        // Увеличение масштаба
        if (event->key() == Qt::Key_Up) {
            for (CustomShape *item : storage.getSelectedItems()) {
                if (!item) continue;
                item->scale(1.15, this);
                update();
            }
        }
        // Уменьшение масштаба
        int delta = 4;
        if (event->key() == Qt::Key_Down) {
            for (CustomShape *item : storage.getSelectedItems()) {
                if (!item) continue;
                item->scale(0.85, this);
                update();
            }
        }
        if (event->key() == Qt::Key_W) {
            for (CustomShape *item : storage.getSelectedItems()) {
                for (CustomShape *item : storage.getSelectedItems()) {
                    if (item)
                        item->move(0, -delta, this);
                }
            }
        }
        if (event->key() == Qt::Key_S) {
            for (CustomShape *item : storage.getSelectedItems()) {
                for (CustomShape *item : storage.getSelectedItems()) {
                    if (item)
                        item->move(0, delta, this);
                }
            }
        }
        if (event->key() == Qt::Key_A) {
            for (CustomShape *item : storage.getSelectedItems()) {
                for (CustomShape *item : storage.getSelectedItems()) {
                    if (item)
                        item->move(-delta, 0, this);
                }
            }
        }
        if (event->key() == Qt::Key_D) {
            for (CustomShape *item : storage.getSelectedItems()) {
                for (CustomShape *item : storage.getSelectedItems()) {
                    if (item)
                        item->move(delta, 0, this);
                }
            }
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        QPointF scenePos = mapToScene(event->pos());

        if (event->button() == Qt::RightButton) {
            if (this->itemMode == "Круг") {
                addCircle(scenePos);
            }
            if (this->itemMode == "Квадрат") {
                addRect(scenePos);
            }
            if (this->itemMode == "Треугольник") {
                addTriangle(scenePos);
            }
            if (this->itemMode == "Линия") {
                addLine(scenePos);
            }
            if (this->itemMode == "Трапеция") {
                addTrapezoid(scenePos);
            }
        }

        if (event->button() == Qt::LeftButton) {
            bool controlPressed = event->modifiers() & Qt::ControlModifier;

            // Если Ctrl не зажат - снимаем выделение со всех
            if (!controlPressed) {
                storage.unselectAll();
            }

            // Пытаемся выбрать объект в точке нажатия
            selectItems(scenePos);
            lastMousePos = scenePos;
        }

        QGraphicsView::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event) override {

        QGraphicsView::mouseMoveEvent(event);
    }

private:
    QPointF lastMousePos;
    std::string itemMode = "Круг";
    double size = 50;
    // IFactoryShape *factory = nullptr; // фабрика для создания фигур

    void addCircle(const QPointF &position) {

        QRect newItemRect = QRect(position.toPoint().x(), position.toPoint().y(), size, size);
        if (CustomShape::isShapeRectInView(newItemRect, this) == false) return;

        MyCircleItem *newCircle = new MyCircleItem(position.toPoint().x(), position.toPoint().y(), size, nullptr);


        newCircle->drawOnScene(position, this, this->color);

        // [!] Добавляем новый элемент в наш storage
        if (newCircle != nullptr)
            storage.addItem(newCircle);
    }

    void addRect(const QPointF &position) {
        QRect newItemRect = QRect(position.toPoint().x(), position.toPoint().y(), size, size);
        if (CustomShape::isShapeRectInView(newItemRect, this) == false) return;

        MySquareItem *newSquare = new MySquareItem(position.toPoint().x(), position.toPoint().y(), size, nullptr);


        newSquare->drawOnScene(position, this, this->color);

        // [!] Добавляем новый элемент в наш storage
        if (newSquare != nullptr)
            storage.addItem(newSquare);
    }

    void addTriangle(const QPointF &position) {
        QRect newItemRect = QRect(position.toPoint().x(), position.toPoint().y(), size, size);
        if (CustomShape::isShapeRectInView(newItemRect, this) == false) return;

        MyTriangleItem *newTriangle = new MyTriangleItem(position.toPoint().x(), position.toPoint().y(), size, nullptr);


        newTriangle->drawOnScene(position, this, this->color);

        // [!] Добавляем новый элемент в наш storage
        if (newTriangle != nullptr)
            storage.addItem(newTriangle);
    }

    void addLine(const QPointF &position) {
        QRect newItemRect = QRect(position.toPoint().x(), position.toPoint().y(), size, size);
        if (CustomShape::isShapeRectInView(newItemRect, this) == false) return;

        MyLineItem *newLine = new MyLineItem(position.toPoint().x(), position.toPoint().y(), size, nullptr);


        newLine->drawOnScene(position, this, this->color);

        // [!] Добавляем новый элемент в наш storage
        if (newLine != nullptr)
            storage.addItem(newLine);
    }

    void addTrapezoid(const QPointF &position) {
        QRect newItemRect = QRect(position.toPoint().x(), position.toPoint().y(), size, size);
        if (CustomShape::isShapeRectInView(newItemRect, this) == false) return;

        MyTrapezoidItem *newTrapezoid = new MyTrapezoidItem(position.toPoint().x(), position.toPoint().y(), size, size, nullptr);


        newTrapezoid->drawOnScene(position, this, this->color);

        // [!] Добавляем новый элемент в наш storage
        if (newTrapezoid != nullptr)
            storage.addItem(newTrapezoid);
    }

    // Функция выбора объектов мышкой
    void selectItems(const QPointF &scenePos) {
        for (int i = 0; i < storage.count(); i++) {
            CustomShape *item = storage.getItem(i);
            if (item->isFocused(scenePos)) {
                item->setMovable(true);
                item->showSelected();
            }
        }
    }

};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    Ui::MainWindow *ui;
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        ui = new Ui::MainWindow;
        ui->setupUi(this);

        QGraphicsScene *scene = new QGraphicsScene(this);
        scene->setSceneRect(0, 0, 700, 700);

        this->view = new MyView(scene, this);

        QSplitter *mainSplitter = new QSplitter(Qt::Vertical, this);


        QPushButton *creatingCircles = new QPushButton("Создавать круги", this);
        connect(creatingCircles, &QPushButton::clicked, [this](){
            this->view->setItemMode("Круг");
        });

        QPushButton *creatingRects = new QPushButton("Создавать квадраты", this);
        connect(creatingRects, &QPushButton::clicked, [this](){
            this->view->setItemMode("Квадрат");
        });

        QPushButton *selectColor = new QPushButton("Выбрать цвет", this);
        connect(selectColor, &QPushButton::clicked, [this](){
            this->view->color = QColorDialog::getColor();
        });

        QPushButton *creatingTriangles = new QPushButton("Создавать треугольники", this);
        connect(creatingTriangles, &QPushButton::clicked, [this](){
            this->view->setItemMode("Треугольник");
        });

        QPushButton *creatingLines = new QPushButton("Создавать линии", this);
        connect(creatingLines, &QPushButton::clicked, [this](){
            this->view->setItemMode("Линия");
        });

        QPushButton *creatingTrapezoids = new QPushButton("Создавать Трапеции", this);
        connect(creatingTrapezoids, &QPushButton::clicked, [this](){
            this->view->setItemMode("Трапеция");
        });


        creatingCircles->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        mainSplitter->addWidget(view);
        mainSplitter->addWidget(creatingCircles);
        mainSplitter->addWidget(creatingRects);
        mainSplitter->addWidget(selectColor);
        mainSplitter->addWidget(creatingTriangles);
        mainSplitter->addWidget(creatingLines);
        mainSplitter->addWidget(creatingTrapezoids);

        setCentralWidget(mainSplitter);
        setWindowTitle("Добавление и перемещение кругов");
        resize(1000, 1000);
    }
private:
    MyView *view;
};

// Точка входа
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

#include "main.moc"
