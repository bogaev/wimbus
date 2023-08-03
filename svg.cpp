#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for (size_t i = 0; i < points_.size(); ++i) {
        if (i > 0) {
            out << " ";
        }
        out << points_[i].x << ","sv << points_[i].y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << " />"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\""sv;
    out << " y=\""sv << pos_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\""sv;
    out << " dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << size_ << "\""sv;
    if (!font_family_.empty()) out << " font-family=\""sv << font_family_ << "\""sv;
    if (!font_weight_.empty()) out << " font-weight=\""sv << font_weight_ << "\""sv;
    out << ">"sv;
    out << data_;
    out << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& obj : objects_) {
        obj->Render(out);
    }
    out << "</svg>"sv;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& linecap) {
    using namespace std::literals;
    if (linecap == StrokeLineCap::BUTT) {
        out << "butt"s;
    }
    else if (linecap == StrokeLineCap::ROUND) {
        out << "round"s;
    }
    else if (linecap == StrokeLineCap::SQUARE) {
        out << "square"s;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& linejoin) {
    using namespace std::literals;
    if (linejoin == StrokeLineJoin::ARCS) {
        out << "arcs"s;
    }
    else if (linejoin == StrokeLineJoin::BEVEL) {
        out << "bevel"s;
    }
    else if (linejoin == StrokeLineJoin::MITER) {
        out << "miter"s;
    }
    else if (linejoin == StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip"s;
    }
    else if (linejoin == StrokeLineJoin::ROUND) {
        out << "round"s;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Rgb& rgb) {
    using namespace std::literals;
    out << "rgb("s << (int)rgb.red << ","s << (int)rgb.green << ","s << (int)rgb.blue << ")"s;
    return out;
}

std::ostream& operator<<(std::ostream& out, const Rgba& rgba) {
    using namespace std::literals;
    out << "rgba("s << (int)rgba.red << ","s << (int)rgba.green << ","s << (int)rgba.blue << ","s << (double)rgba.opacity << ")"s;
    return out;
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(ColorPrinter{ out }, color);
    return out;
}

}  // namespace svg