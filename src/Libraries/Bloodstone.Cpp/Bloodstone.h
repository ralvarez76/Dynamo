
#ifdef BLOODSTONE_EXPORTS
#define BLOODSTONE_API __declspec(dllexport)
#else
#define BLOODSTONE_API __declspec(dllimport)
#endif

namespace Gen = System::Collections::Generic;
namespace Ds = Autodesk::DesignScript::Interfaces;

namespace Dynamo { namespace Bloodstone {

    class IGraphicsContext;
    class IShaderProgram;
    class IVertexBuffer;
    class GeometryData;
    class NodeGeometries;
    class BoundingBox;
    ref class Scene;

    public ref class NodeDetails
    {
    public:
        NodeDetails(int depth, Ds::IRenderPackage^ renderPackage)
        {
            this->depth = depth;
            this->renderPackage = renderPackage;
            this->red = this->green = this->blue = 1.0;
        }

        void SetColor(double red, double green, double blue)
        {
            this->red   = ((red < 0.0 ? 0.0 : red) > 1.0 ? 1.0 : red);
            this->green = ((green < 0.0 ? 0.0 : green) > 1.0 ? 1.0 : green);
            this->blue  = ((blue < 0.0 ? 0.0 : blue) > 1.0 ? 1.0 : blue);
        }

        property int Depth
        {
            int get() { return this->depth; }
        }

        property Ds::IRenderPackage^ RenderPackage
        {
            Ds::IRenderPackage^ get() { return this->renderPackage; }
        }

        property double Red     { double get() { return this->red;   } }
        property double Green   { double get() { return this->green; } }
        property double Blue    { double get() { return this->blue;  } }

    private:
        int depth;
        double red, green, blue;
        Ds::IRenderPackage^ renderPackage;
    };

    typedef Gen::IEnumerable<Gen::KeyValuePair<System::String^, NodeDetails^>> NodeDetailsType;

    public ref class Visualizer
    {
    public:

        // Static class methods
        static System::IntPtr Create(System::IntPtr hwndParent, int width, int height);
        static void Destroy(void);
        static Visualizer^ CurrentInstance(void);
        static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        // Public class methods.
        void ShowWindow(bool show);
        void RequestFrameUpdate(void);
        HWND GetWindowHandle(void);
        Scene^ GetScene(void);
        IGraphicsContext* GetGraphicsContext(void);

    private:

        // Private class instance methods.
        Visualizer();
        void Initialize(HWND hWndParent, int width, int height);
        void Uninitialize(void);
        LRESULT ProcessMouseMessage(UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT ProcessMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

        // Static class data member.
        static Visualizer^ mVisualizer = nullptr;

        // Class instance data members.
        HWND mhWndVisualizer;
        Scene^ mpScene;
        IGraphicsContext* mpGraphicsContext;
    };

    public ref class Scene
    {
    public:
        Scene(Visualizer^ visualizer);
        void Initialize(int width, int height);
        void Destroy(void);
        void RenderScene(void);
        void UpdateNodeDetails(NodeDetailsType^ nodeDetails);
        void RemoveNodeGeometries(Gen::IEnumerable<System::String^>^ nodes);

    private:
        void UpdateNodeGeometries(NodeDetailsType^ nodeDetails);
        void RenderGeometries(const std::vector<NodeGeometries *>& geometries);

    private:
        int mAlphaParamIndex;
        int mColorParamIndex;
        int mControlParamsIndex;
        IShaderProgram* mpShaderProgram;

        Visualizer^ mVisualizer;
        std::map<std::wstring, NodeGeometries*>* mpNodeGeometries;
    };
} }