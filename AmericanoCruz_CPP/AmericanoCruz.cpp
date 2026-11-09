// Americano Cruz - Implementacion C++ del modelo de diseno
// Grupo #1 - Ingenieria de Software II

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <iomanip>
using namespace std;

class Usuario {
private:
    int idUsuario;
    string nombre;
    string rol;
    bool estado;
public:
    Usuario(int id=0, string n="", string r="cliente", bool e=true)
        : idUsuario(id), nombre(n), rol(r), estado(e) {}
    int getIdUsuario() const { return idUsuario; }
    string getNombre() const { return nombre; }
    string getRol() const { return rol; }
    bool getEstado() const { return estado; }
};

class Producto {
private:
    int idProducto;
    string codigo;
    string nombre;
    double precio;
    int existencia;
public:
    Producto(int id=0, string c="", string n="", double p=0, int e=0)
        : idProducto(id), codigo(c), nombre(n), precio(p), existencia(e) {}
    int getIdProducto() const { return idProducto; }
    string getCodigo() const { return codigo; }
    string getNombre() const { return nombre; }
    double getPrecio() const { return precio; }
    int getExistencia() const { return existencia; }
    void aumentarExistencia(int cantidad) {
        if (cantidad > 0) existencia += cantidad;
    }
    bool disminuirExistencia(int cantidad) {
        if (cantidad <= 0 || cantidad > existencia) return false;
        existencia -= cantidad;
        return true;
    }
};

class FormaPago {
private:
    int idFormaPago;
    string tipo;
    double monto;
public:
    FormaPago(int id=0, string t="", double m=0)
        : idFormaPago(id), tipo(t), monto(m) {}
    string getTipo() const { return tipo; }
    double getMonto() const { return monto; }
};

class DetalleVenta {
private:
    Producto producto;
    int cantidad;
    double precioUnitario;
    double subtotal;
public:
    DetalleVenta(const Producto& p, int c)
        : producto(p), cantidad(c),
          precioUnitario(p.getPrecio()),
          subtotal(c * p.getPrecio()) {}
    const Producto& getProducto() const { return producto; }
    int getCantidad() const { return cantidad; }
    double getSubtotal() const { return subtotal; }
};

class Venta {
private:
    int idVenta;
    string fecha;
    double total;
    string estado;
    FormaPago formaPago;
    vector<DetalleVenta> detalles;
public:
    Venta(int id=0, string f="", FormaPago fp=FormaPago())
        : idVenta(id), fecha(f), total(0), estado("PENDIENTE"), formaPago(fp) {}
    void agregarDetalle(const DetalleVenta& d) {
        detalles.push_back(d);
        calcularTotal();
    }
    double calcularTotal() {
        total = 0;
        for (const auto& d : detalles) total += d.getSubtotal();
        return total;
    }
    void confirmar() { estado = "CONFIRMADA"; }
    int getIdVenta() const { return idVenta; }
    string getFecha() const { return fecha; }
    double getTotal() const { return total; }
    string getEstado() const { return estado; }
    const FormaPago& getFormaPago() const { return formaPago; }
    const vector<DetalleVenta>& getDetalles() const { return detalles; }
};

class MovimientoInventario {
private:
    int idMovimiento;
    int idProducto;
    string fecha;
    string tipo;
    int cantidad;
public:
    MovimientoInventario(int id=0, int prod=0, string f="", string t="", int c=0)
        : idMovimiento(id), idProducto(prod), fecha(f), tipo(t), cantidad(c) {}
};

class ProductoRepository {
private:
    vector<Producto> productos;
public:
    void agregar(const Producto& p) { productos.push_back(p); }
    Producto* buscarPorId(int id) {
        for (auto& p : productos)
            if (p.getIdProducto() == id) return &p;
        return nullptr;
    }
    const vector<Producto>& listar() const { return productos; }
};

class VentaRepository {
private:
    vector<Venta> ventas;
public:
    void guardar(const Venta& v) { ventas.push_back(v); }
};

class MovimientoRepository {
private:
    vector<MovimientoInventario> movimientos;
public:
    void guardar(const MovimientoInventario& m) { movimientos.push_back(m); }
};

class InventarioService {
private:
    ProductoRepository& productoRepository;
    MovimientoRepository& movimientoRepository;
    int siguienteMovimiento;
public:
    InventarioService(ProductoRepository& p, MovimientoRepository& m)
        : productoRepository(p), movimientoRepository(m), siguienteMovimiento(1) {}

    bool validarStock(const vector<pair<int,int>>& items) {
        for (const auto& item : items) {
            Producto* p = productoRepository.buscarPorId(item.first);
            if (!p || p->getExistencia() < item.second) return false;
        }
        return true;
    }

    bool registrarEntrada(int idProducto, int cantidad, const string& fecha) {
        Producto* p = productoRepository.buscarPorId(idProducto);
        if (!p || cantidad <= 0) return false;
        p->aumentarExistencia(cantidad);
        movimientoRepository.guardar(
            MovimientoInventario(siguienteMovimiento++, idProducto, fecha, "ENTRADA", cantidad)
        );
        return true;
    }

    bool descontarStock(const vector<pair<int,int>>& items, const string& fecha) {
        if (!validarStock(items)) return false;
        for (const auto& item : items) {
            Producto* p = productoRepository.buscarPorId(item.first);
            p->disminuirExistencia(item.second);
            movimientoRepository.guardar(
                MovimientoInventario(siguienteMovimiento++, item.first, fecha, "SALIDA", item.second)
            );
        }
        return true;
    }
};

class VentaService {
private:
    ProductoRepository& productoRepository;
    VentaRepository& ventaRepository;
    InventarioService& inventarioService;
    int siguienteVenta;
public:
    VentaService(ProductoRepository& p, VentaRepository& v, InventarioService& i)
        : productoRepository(p), ventaRepository(v), inventarioService(i), siguienteVenta(1) {}

    bool procesarVenta(const vector<pair<int,int>>& items,
                       const string& fecha,
                       const FormaPago& formaPago,
                       Venta& salida) {
        if (items.empty() || !inventarioService.validarStock(items)) return false;

        Venta venta(siguienteVenta++, fecha, formaPago);
        for (const auto& item : items) {
            Producto* p = productoRepository.buscarPorId(item.first);
            if (!p) return false;
            venta.agregarDetalle(DetalleVenta(*p, item.second));
        }

        if (!inventarioService.descontarStock(items, fecha)) return false;

        venta.calcularTotal();
        venta.confirmar();
        ventaRepository.guardar(venta);
        salida = venta;
        return true;
    }
};

class VentaController {
private:
    VentaService& ventaService;
public:
    VentaController(VentaService& s) : ventaService(s) {}
    bool registrarVenta(const vector<pair<int,int>>& items,
                        const string& fecha,
                        const FormaPago& pago,
                        Venta& venta) {
        return ventaService.procesarVenta(items, fecha, pago, venta);
    }
};

class InventarioController {
private:
    InventarioService& inventarioService;
public:
    InventarioController(InventarioService& s) : inventarioService(s) {}
    bool registrarEntrada(int idProducto, int cantidad, const string& fecha) {
        return inventarioService.registrarEntrada(idProducto, cantidad, fecha);
    }
};

class VentaView {
public:
    void mostrarComprobante(const Venta& venta) const {
        cout << "\n========== AMERICANO CRUZ ==========\n";
        cout << "Venta #" << venta.getIdVenta() << "\n";
        for (const auto& d : venta.getDetalles()) {
            cout << d.getProducto().getNombre()
                 << " x" << d.getCantidad()
                 << "  L " << fixed << setprecision(2)
                 << d.getSubtotal() << "\n";
        }
        cout << "Total: L " << venta.getTotal() << "\n";
        cout << "Pago: " << venta.getFormaPago().getTipo() << "\n";
        cout << "Estado: " << venta.getEstado() << "\n";
    }
};

int main() {
    cout << "SISTEMA AMERICANO CRUZ - C++\n";

    ProductoRepository productos;
    VentaRepository ventas;
    MovimientoRepository movimientos;

    productos.agregar(Producto(1, "ZAP-001", "Zapato On Cloud Coral", 1290.00, 5));
    productos.agregar(Producto(2, "VES-001", "Vestido Cafe", 650.00, 3));

    InventarioService inventarioService(productos, movimientos);
    VentaService ventaService(productos, ventas, inventarioService);
    VentaController ventaController(ventaService);
    InventarioController inventarioController(inventarioService);

    inventarioController.registrarEntrada(1, 2, "08/11/2026");

    vector<pair<int,int>> items = {{1,1},{2,1}};
    FormaPago pago(1, "Efectivo", 1940.00);
    Venta venta;

    if (ventaController.registrarVenta(items, "08/11/2026", pago, venta)) {
        VentaView().mostrarComprobante(venta);
    }

    return 0;
}