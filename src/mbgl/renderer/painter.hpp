#ifndef MBGL_RENDERER_PAINTER
#define MBGL_RENDERER_PAINTER

#include <mbgl/map/transform_state.hpp>
#include <mbgl/map/map_context.hpp>

#include <mbgl/renderer/frame_history.hpp>
#include <mbgl/renderer/bucket.hpp>

#include <mbgl/geometry/vao.hpp>
#include <mbgl/geometry/static_vertex_buffer.hpp>

#include <mbgl/renderer/gl_config.hpp>

#include <mbgl/style/types.hpp>

#include <mbgl/platform/gl.hpp>

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/chrono.hpp>

#include <array>
#include <vector>
#include <set>

namespace mbgl {

class Style;
class StyleLayer;
class Tile;
class SpriteAtlas;
class GlyphAtlas;
class LineAtlas;
class Source;
struct FrameData;


class DebugBucket;
class FillBucket;
class LineBucket;
class SymbolBucket;
class RasterBucket;

struct RasterProperties;

class SDFShader;
class PlainShader;
class OutlineShader;
class LineShader;
class LinejoinShader;
class LineSDFShader;
class LinepatternShader;
class PatternShader;
class IconShader;
class RasterShader;
class SDFGlyphShader;
class SDFIconShader;
class DotShader;
class CollisionBoxShader;

struct ClipID;

struct RenderItem {
    inline RenderItem(const StyleLayer& layer_,
                      const Tile* tile_ = nullptr,
                      Bucket* bucket_ = nullptr)
        : tile(tile_), bucket(bucket_), layer(layer_) {
    }

    const Tile* const tile;
    Bucket* const bucket;
    const StyleLayer& layer;
};

class Painter : private util::noncopyable {
public:
    Painter(MapData& data);
    ~Painter();

    void setup();

    // Renders the backdrop of the OpenGL view. This also paints in areas where we don't have any
    // tiles whatsoever.
    void clear();

    // Updates the default matrices to the current viewport dimensions.
    void changeMatrix();

    void render(const Style& style,
                TransformState state,
                const FrameData& frame,
                const TimePoint& time);

    // Renders debug information for a tile.
    void renderTileDebug(const Tile& tile);

    // Renders the red debug frame around a tile, visualizing its perimeter.
    void renderDebugFrame(const mat4 &matrix);

    void renderDebugText(DebugBucket& bucket, const mat4 &matrix);
    void renderFill(FillBucket& bucket, const StyleLayer &layer_desc, const TileID& id, const mat4 &matrix);
    void renderLine(LineBucket& bucket, const StyleLayer &layer_desc, const TileID& id, const mat4 &matrix);
    void renderSymbol(SymbolBucket& bucket, const StyleLayer &layer_desc, const TileID& id, const mat4 &matrix);
    void renderRaster(RasterBucket& bucket, const StyleLayer &layer_desc, const TileID& id, const mat4 &matrix);
    void renderBackground(const StyleLayer &layer_desc);

    float saturationFactor(float saturation);
    float contrastFactor(float contrast);
    std::array<float, 3> spinWeights(float spin_value);

    void preparePrerender(RasterBucket &bucket);

    void renderPrerenderedTexture(RasterBucket &bucket, const mat4 &matrix, const RasterProperties& properties);

    void createPrerendered(RasterBucket& bucket, const StyleLayer &layer_desc, const TileID& id);

    // Adjusts the dimensions of the OpenGL viewport
    void resize();

    // Changes whether debug information is drawn onto the map
    void setDebug(bool enabled);

    // Configures the painter strata that is used for early z-culling of fragments.
    void setStrata(float strata);

    void drawClippingMasks(const std::set<Source*>&);
    void drawClippingMask(const mat4& matrix, const ClipID& clip);

    void resetFramebuffer();
    void bindFramebuffer();
    void pushFramebuffer();
    GLuint popFramebuffer();
    void discardFramebuffers();

    bool needsAnimation() const;

    void updateRenderOrder(const Style& style);

private:
    void setupShaders();
    mat4 translatedMatrix(const mat4& matrix, const std::array<float, 2> &translation, const TileID &id, TranslateAnchorType anchor);

    template <class Iterator>
    void renderPass(RenderPass,
                    Iterator it, Iterator end,
                    std::size_t i, int8_t iIncrement,
                    const float strataThickness);

    void prepareTile(const Tile& tile);

    template <typename BucketProperties, typename StyleProperties>
    void renderSDF(SymbolBucket &bucket,
                   const TileID &id,
                   const mat4 &matrixSymbol,
                   const BucketProperties& bucketProperties,
                   const StyleProperties& styleProperties,
                   float scaleDivisor,
                   std::array<float, 2> texsize,
                   SDFShader& sdfShader,
                   void (SymbolBucket::*drawSDF)(SDFShader&));

public:
    void useProgram(uint32_t program);
    void lineWidth(float lineWidth);

public:
    mat4 projMatrix;
    mat4 nativeMatrix;
    mat4 extrudeMatrix;

    // used to composite images and flips the geometry upside down
    const mat4 flipMatrix = []{
        mat4 flip;
        matrix::ortho(flip, 0, 4096, -4096, 0, 0, 1);
        matrix::translate(flip, flip, 0, -4096, 0);
        return flip;
    }();

    const mat4 identityMatrix = []{
        mat4 identity;
        matrix::identity(identity);
        return identity;
    }();

private:
    MapData& data;

    TransformState state;
    FrameData frame;

    bool debug = false;
    int indent = 0;

    gl::Config config;

    uint32_t gl_program = 0;
    float gl_lineWidth = 0;
    std::array<uint16_t, 2> gl_viewport = {{ 0, 0 }};
    float strata = 0;
    RenderPass pass = RenderPass::Opaque;
    const float strata_epsilon = 1.0f / (1 << 16);
    Color background = {{ 0, 0, 0, 0 }};

    std::vector<RenderItem> order;

public:
    FrameHistory frameHistory;

    SpriteAtlas* spriteAtlas;
    GlyphAtlas* glyphAtlas;
    LineAtlas* lineAtlas;

    std::unique_ptr<PlainShader> plainShader;
    std::unique_ptr<OutlineShader> outlineShader;
    std::unique_ptr<LineShader> lineShader;
    std::unique_ptr<LineSDFShader> linesdfShader;
    std::unique_ptr<LinepatternShader> linepatternShader;
    std::unique_ptr<PatternShader> patternShader;
    std::unique_ptr<IconShader> iconShader;
    std::unique_ptr<RasterShader> rasterShader;
    std::unique_ptr<SDFGlyphShader> sdfGlyphShader;
    std::unique_ptr<SDFIconShader> sdfIconShader;
    std::unique_ptr<DotShader> dotShader;
    std::unique_ptr<CollisionBoxShader> collisionBoxShader;

    StaticVertexBuffer backgroundBuffer = {
        { -1, -1 }, { 1, -1 },
        { -1,  1 }, { 1,  1 }
    };

    VertexArrayObject backgroundArray;

    // Set up the stencil quad we're using to generate the stencil mask.
    StaticVertexBuffer tileStencilBuffer = {
        // top left triangle
        { 0, 0 },
        { 4096, 0 },
        { 0, 4096 },

        // bottom right triangle
        { 4096, 0 },
        { 0, 4096 },
        { 4096, 4096 },
    };

    VertexArrayObject coveringPlainArray;
    VertexArrayObject coveringRasterArray;

    // Set up the tile boundary lines we're using to draw the tile outlines.
    StaticVertexBuffer tileBorderBuffer = {
        { 0, 0 },
        { 4096, 0 },
        { 4096, 4096 },
        { 0, 4096 },
        { 0, 0 },
    };

    VertexArrayObject tileBorderArray;

    // Framebuffer management
    std::vector<GLuint> fbos;
    std::vector<GLuint> fbos_color;
    GLuint fbo_depth_stencil;
    int fbo_level = -1;
    bool fbo_depth_stencil_valid = false;

};

}

#endif
