#include <iostream>
#include <Windows.h>
#include <format>
#include <queue>
#include <set>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <unordered_map>
#include "tinyxml2.h"

#define FILE_NAME "tree.xml"

class BinaryTree {
private:
    struct Top {
        int digital_top;
        std::unique_ptr<Top> left;
        std::unique_ptr<Top> right;
        Top() : digital_top(0), left(nullptr), right(nullptr) {}
        ~Top() {}
    };

    std::unique_ptr<Top> root;

    int generate_random_number(int min_value, int max_value) {
        static std::mt19937_64 engine{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(min_value, max_value);
        return dist(engine);
    }

    void resizeRange(int& min, int& max, int size) {
        min = max + 1;
        max += size + 1;
    }

    // XML serialization
    void saveNodeToXML(tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* parent, const Top* node) const {
        if (!node) {
            return;
        }
        auto* nodeElement = doc.NewElement("Node");
        nodeElement->SetAttribute("value", node->digital_top);
        parent->InsertEndChild(nodeElement);
        {
            auto* leftElement = doc.NewElement("Left");
            nodeElement->InsertEndChild(leftElement);
            if (node->left) saveNodeToXML(doc, leftElement, node->left.get());
        }
        {
            auto* rightElement = doc.NewElement("Right");
            nodeElement->InsertEndChild(rightElement);
            if (node->right) saveNodeToXML(doc, rightElement, node->right.get());
        }
    }

    std::unique_ptr<Top> loadNodeFromXML(tinyxml2::XMLElement* element) {
        if (!element || !element->FirstChildElement("Node")) return nullptr;
        auto* nodeElement = element->FirstChildElement("Node");
        auto node = std::make_unique<Top>();
        node->digital_top = nodeElement->IntAttribute("value");

        auto* leftElement = nodeElement->FirstChildElement("Left");
        if (leftElement) node->left = loadNodeFromXML(leftElement);

        auto* rightElement = nodeElement->FirstChildElement("Right");
        if (rightElement) node->right = loadNodeFromXML(rightElement);

        return node;
    }

    // Проверка heap-order property
    bool isMaxHeap(const Top* node) const {
        if (!node) return true;
        if (node->left) {
            if (node->digital_top < node->left->digital_top) return false;
            if (!isMaxHeap(node->left.get())) return false;
        }
        if (node->right) {
            if (node->digital_top < node->right->digital_top) return false;
            if (!isMaxHeap(node->right.get())) return false;
        }
        return true;
    }

    bool isMinHeap(const Top* node) const {
        if (!node) return true;
        if (node->left) {
            if (node->digital_top > node->left->digital_top) return false;
            if (!isMinHeap(node->left.get())) return false;
        }
        if (node->right) {
            if (node->digital_top > node->right->digital_top) return false;
            if (!isMinHeap(node->right.get())) return false;
        }
        return true;
    }

    // Проверка полноты (complete binary tree): пункты 2 и 3
    bool isCompleteBinaryTree(const Top* root) const {
        if (!root) return true;
        std::queue<const Top*> q;
        q.push(root);
        bool mustBeLeaf = false;
        while (!q.empty()) {
            const Top* node = q.front(); q.pop();
            // Левый потомок
            if (node->left) {
                if (mustBeLeaf) return false;
                q.push(node->left.get());
            }
            else {
                mustBeLeaf = true;
            }
            // Правый потомок
            if (node->right) {
                if (mustBeLeaf) return false;
                q.push(node->right.get());
            }
            else {
                mustBeLeaf = true;
            }
        }
        return true;
    }

    bool isLeftLessThanRight(const Top* node) const {
        if (!node) return true;
        bool currentValid = true;
        if (node->left && node->right) {
            if (node->left->digital_top >= node->right->digital_top) {
                currentValid = false;
            }
        }
        return currentValid && isLeftLessThanRight(node->left.get()) && isLeftLessThanRight(node->right.get());
    }

    void collectSubtreeNodes(const Top* node, std::vector<int>& nodes) const {
        if (!node) return;
        nodes.push_back(node->digital_top);
        collectSubtreeNodes(node->left.get(), nodes);
        collectSubtreeNodes(node->right.get(), nodes);
    }

    // Печать пирамидальных поддеревьев с учётом новых проверок
    void findAndPrintHeapSubtrees(const Top* node, int& subtree_counter) const {
        if (!node) return;

        bool isMax = isMaxHeap(node);
        bool isMin = isMinHeap(node);
        bool complete = isCompleteBinaryTree(node);
        bool left_mini = isLeftLessThanRight(node);

        if ((isMax || isMin) && complete && left_mini) {
            std::vector<int> subtree_nodes;
            collectSubtreeNodes(node, subtree_nodes);

            std::cout << subtree_counter << ". Поддерево с корнем " << node->digital_top;
            if (isMax && isMin) {
                std::cout << " (max-пирамида и min-пирамида)";
            }
            else if (isMax) {
                std::cout << " (max-пирамида)";
            }
            else {
                std::cout << " (min-пирамида)";
            }
            std::cout << "\n   Вершины: ";
            for (size_t i = 0; i < subtree_nodes.size(); ++i) {
                std::cout << subtree_nodes[i] << (i + 1 < subtree_nodes.size() ? ", " : "");
            }
            std::cout << std::endl;
            ++subtree_counter;
        }

        findAndPrintHeapSubtrees(node->left.get(), subtree_counter);
        findAndPrintHeapSubtrees(node->right.get(), subtree_counter);
    }

    // Печать непирамидальных поддеревьев
    void findAndPrintNonHeapSubtrees(const Top* node, int& subtree_counter) const {
        if (!node) return;

        bool isMax = isMaxHeap(node);
        bool isMin = isMinHeap(node);
        bool complete = isCompleteBinaryTree(node);
        bool left_mini = isLeftLessThanRight(node);

        if (!( (isMax || isMin) && complete && left_mini)) {
            std::vector<int> subtree_nodes;
            collectSubtreeNodes(node, subtree_nodes);

            std::cout << subtree_counter << ". Поддерево с корнем " << node->digital_top
                << " (не является пирамидой)\n";
            std::cout << "   Вершины: ";
            for (size_t i = 0; i < subtree_nodes.size(); ++i) {
                std::cout << subtree_nodes[i] << (i + 1 < subtree_nodes.size() ? ", " : "");
            }
            std::cout << std::endl;
            ++subtree_counter;
        }

        findAndPrintNonHeapSubtrees(node->left.get(), subtree_counter);
        findAndPrintNonHeapSubtrees(node->right.get(), subtree_counter);
    }

    // Подсчет статистики с учётом полноты
    void checkHeap_notHeap(const Top* node, int& heap_count, int& nonheap_count) const {
        if (!node) return;
        bool isMax = isMaxHeap(node);
        bool isMin = isMinHeap(node);
        bool complete = isCompleteBinaryTree(node);
        bool left_mini = isLeftLessThanRight(node);

        if ((isMax || isMin) && complete && left_mini) {
            ++heap_count;
        }
        else {
            ++nonheap_count;
        }

        checkHeap_notHeap(node->left.get(), heap_count, nonheap_count);
        checkHeap_notHeap(node->right.get(), heap_count, nonheap_count);
    }

    int countNodes(const Top* node) const {
        if (!node) return 0;
        return 1 + countNodes(node->left.get()) + countNodes(node->right.get());
    }

public:
    BinaryTree() { std::cout << "Объект дерева инициализирован" << std::endl; }
    ~BinaryTree() { std::cout << "Дерево деинициализировано" << std::endl; }

    void generate(int top_count) {
        std::set<int> dubl_digital;
        int min_value = 1, max_value = 5;
        if (top_count < 1) {
            root.reset();
            return;
        }
        root = std::make_unique<Top>();
        int topD = generate_random_number(min_value, max_value);
        root->digital_top = topD;
        dubl_digital.insert(topD);
        resizeRange(min_value, max_value, 10);
        int current_count = 1;
        std::queue<Top*> nodes_queue;
        nodes_queue.push(root.get());

        while (current_count < top_count && !nodes_queue.empty()) {
            Top* current = nodes_queue.front();
            nodes_queue.pop();
            if (current_count < top_count) {
                current->left = std::make_unique<Top>();
            hh_gen:
                {
                    int top_dig = generate_random_number(min_value, max_value);
                    if (dubl_digital.contains(top_dig)) goto hh_gen;
                    current->left->digital_top = top_dig;
                    nodes_queue.push(current->left.get());
                    ++current_count;
                    dubl_digital.insert(top_dig);
                }
                resizeRange(min_value, max_value, 5);
            }
            if (current_count < top_count) {
            hh_gen2:
                {
                    int top_dig = generate_random_number(min_value, max_value);
                    if (dubl_digital.contains(top_dig)) goto hh_gen2;
                    current->right = std::make_unique<Top>();
                    current->right->digital_top = top_dig;
                    nodes_queue.push(current->right.get());
                    ++current_count;
                    dubl_digital.insert(top_dig);
                }
                resizeRange(min_value, max_value, 5);
            }
        }
    }

    void generateRnd(int top_count) {
        std::set<int> dubl_digital;
        int min_value = 1, max_value = 120000;
        if (top_count < 1) {
            root.reset();
            return;
        }
        root = std::make_unique<Top>();
        int topD = generate_random_number(min_value, max_value);
        root->digital_top = topD;
        dubl_digital.insert(topD);

        int current_count = 1;
        std::queue<Top*> nodes_queue;
        nodes_queue.push(root.get());

        while (current_count < top_count && !nodes_queue.empty()) {
            Top* current = nodes_queue.front();
            nodes_queue.pop();
            if (current_count < top_count) {
                current->left = std::make_unique<Top>();
            hh_gen:
                {
                    int top_dig = generate_random_number(min_value, max_value);
                    if (dubl_digital.contains(top_dig)) goto hh_gen;
                    current->left->digital_top = top_dig;
                    nodes_queue.push(current->left.get());
                    ++current_count;
                    dubl_digital.insert(top_dig);
                }
            }
            if (current_count < top_count) {
            hh_gen2:
                {
                    int top_dig = generate_random_number(min_value, max_value);
                    if (dubl_digital.contains(top_dig)) goto hh_gen2;
                    current->right = std::make_unique<Top>();
                    current->right->digital_top = top_dig;
                    nodes_queue.push(current->right.get());
                    ++current_count;
                    dubl_digital.insert(top_dig);
                }
            }
        }
    }

    void printBranch(const Top* node, std::string prefix, bool isLeft) {
        if (!node) return;
        std::cout << prefix << (isLeft ? "|____ " : "|-- ") << node->digital_top << std::endl;
        std::string newPrefix = prefix + (isLeft ? "    " : "|   ");
        if (node->right) printBranch(node->right.get(), newPrefix, false);
        if (node->left)  printBranch(node->left.get(), newPrefix, true);
    }

    void PrintTree() {
        if (!root) {
            std::cout << "Дерево пустое" << std::endl;
            return;
        }
        std::cout << root->digital_top << std::endl;
        if (root->right) printBranch(root->right.get(), "", false);
        if (root->left)  printBranch(root->left.get(), "", true);
    }

    void printAllSubtrees() {
        if (!root) {
            std::cout << "Дерево пустое" << std::endl;
            return;
        }
        std::cout << "Пирамидальные поддеревья:" << std::endl;
        int pyramid_counter = 1;
        findAndPrintHeapSubtrees(root.get(), pyramid_counter);

        std::cout << std::endl << "Не пирамидальные поддеревья:" << std::endl;
        int non_pyramid_counter = 1;
        findAndPrintNonHeapSubtrees(root.get(), non_pyramid_counter);
    }

    void printStatistics() {
        int pyramid_counter = 0;
        int non_pyramid_counter = 0;
        checkHeap_notHeap(root.get(), pyramid_counter, non_pyramid_counter);
        std::cout << std::format("являются пирамидой: {} | не являются пирамидой: {}",
            pyramid_counter, non_pyramid_counter)
            << std::endl;
    }

    void clearTree() {
        root.reset();
    }

    int getTreeSize() {
        return countNodes(root.get());
    }

    void save(const std::string& filename) {
        tinyxml2::XMLDocument doc;
        auto* rootElement = doc.NewElement("BinaryTree");
        doc.InsertFirstChild(rootElement);
        if (root) saveNodeToXML(doc, rootElement, root.get());
        if (doc.SaveFile(filename.c_str()) == tinyxml2::XML_SUCCESS) {
            std::cout << "Дерево успешно сохранено в файл " << filename << std::endl;
        }
        else {
            std::cout << "Ошибка при сохранении дерева в файл " << filename << std::endl;
        }
    }

    void load(const std::string& filename) {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
            std::cout << "Ошибка при загрузке файла " << filename << std::endl;
            return;
        }
        auto* rootElement = doc.FirstChildElement("BinaryTree");
        if (!rootElement) {
            std::cout << "Неверный формат XML файла" << std::endl;
            return;
        }
        root = loadNodeFromXML(rootElement);
        std::cout << "Дерево успешно загружено из файла " << filename << std::endl;
    }
};

void printDuration(std::chrono::microseconds duration) {
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
    duration -= sec;
    auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    duration -= millisec;
    auto microsec = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    std::cout << "Время выполнения(sec/milisec/microsec): "
        << sec.count() << "."
        << std::setfill('0') << std::setw(3) << millisec.count() << "."
        << std::setfill('0') << std::setw(3) << microsec.count()
        << std::endl;
}

int main() {
    std::srand(std::time(nullptr));
    setlocale(0, "RU");

    auto tree = std::make_unique<BinaryTree>();
    tree->load(FILE_NAME);

    std::cout << "1. Генерировать автоматически N'го размера min->max" << std::endl;
    std::cout << "2. Генерировать автоматически N'го размера random" << std::endl;
    std::cout << "3. Определить поддеревья на пирамиды" << std::endl;
    std::cout << "4. Вывести" << std::endl;
    std::cout << "5. Сохранить сгенерированное дерево" << std::endl;
    std::cout << "6. Перезагрузить дерево из файла" << std::endl;
    std::cout << "7. Выход" << std::endl;

    while (true) {
        std::cout << "Введите команду: " << std::endl;
        int command = 0;
        std::cin >> command;
        bool exit_destructor = false;

        switch (command) {
        case 1: {
            int countTops;
            std::cout << "Введите длину дерева: ";
            std::cin >> countTops;
            std::cout << std::format("Размер дерева: {}", tree->getTreeSize()) << std::endl;
            tree->clearTree();
            std::cout << std::format("Размер дерева: {}", tree->getTreeSize()) << std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            tree->generate(countTops);
            auto stop = std::chrono::high_resolution_clock::now();
            std::cout << "Дерево сгенерировано" << std::endl;
            printDuration(std::chrono::duration_cast<std::chrono::microseconds>(stop - start));
            break;
        }
        case 2: {
            int countTops;
            std::cout << "Введите длину дерева: ";
            std::cin >> countTops;
            std::cout << std::format("Размер дерева: {}", tree->getTreeSize()) << std::endl;
            tree->clearTree();
            std::cout << std::format("Размер дерева: {}", tree->getTreeSize()) << std::endl;
            auto start = std::chrono::high_resolution_clock::now();
            tree->generateRnd(countTops);
            auto stop = std::chrono::high_resolution_clock::now();
            std::cout << "Дерево сгенерировано, рандомный порядок" << std::endl;
            printDuration(std::chrono::duration_cast<std::chrono::microseconds>(stop - start));
            break;
        }
        case 3: {
            std::cout << "1. Подсчёт + Вывод с вершинами" << std::endl;
            std::cout << "2. Подсчет и статистика" << std::endl;
            int comm; std::cin >> comm;
            if (comm == 1) {
                auto start = std::chrono::high_resolution_clock::now();
                tree->printAllSubtrees();
                auto stop = std::chrono::high_resolution_clock::now();
                printDuration(std::chrono::duration_cast<std::chrono::microseconds>(stop - start));
            }
            else if (comm == 2) {
                auto start = std::chrono::high_resolution_clock::now();
                tree->printStatistics();
                auto stop = std::chrono::high_resolution_clock::now();
                printDuration(std::chrono::duration_cast<std::chrono::microseconds>(stop - start));
            }
            break;
        }
        case 4:
            std::cout << "Дерево: " << std::endl;
            tree->PrintTree();
            break;
        case 5:
            tree->save(FILE_NAME);
            break;
        case 6:
            tree->load(FILE_NAME);
            break;
        case 7:
            exit_destructor = true;
            break;
        default:
            std::cout << "Нет такой команды" << std::endl;
        }

        if (exit_destructor) break;
    }

    return 0;
}
