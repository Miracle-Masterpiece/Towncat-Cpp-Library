#include <cpp/lang/utils/images/image_tree.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <new>

namespace tc
{

namespace texturing
{

    node::node() : 
    m_allocator(nullptr), 
    m_left(), 
    m_right(), 
    m_rect(), 
    m_ID(NULL_ID) {

    }

    node::node(int w, int h, tca::allocator* allocator) : node({0, 0, w, h}, allocator) {

    }

    node::node(const rect& r, tca::allocator* allocator) : 
    m_allocator(allocator), 
    m_left(), 
    m_right(), 
    m_rect(r), 
    m_ID(NULL_ID) {

    }

    node::node(node&& n) :
    m_allocator(n.m_allocator),
    m_left(std::move(n.m_left)),
    m_right(std::move(n.m_right)),
    m_rect(n.m_rect),
    m_ID(n.m_ID) {
        n.m_ID          = NULL_ID;
    }
    
    node& node::operator= (node&& n) {
        if (&n != this) {
            m_left      = std::move(n.m_left);
            m_right     = std::move(n.m_right);
            m_rect      = n.m_rect;
            m_ID        = n.m_ID;
            n.m_ID      = NULL_ID;
        }
        return *this;
    }
    
    node::~node() {
        
    }

    bool node::is_leaf() const {
        return m_left == nullptr && m_right == nullptr;
    }

    node* node::put_image(int w, int h, std::size_t imageID) {
        
        if (m_allocator == nullptr)
            throw_except<illegal_state_exception>("allocator must be != null");
        
        if (w < 0 || h < 0)
            throw_except<illegal_argument_exception>("Width or height must be >= 0");

        if (!is_leaf()){
			node* n = m_left->put_image(w, h, imageID);
			if (n != nullptr)
				return n;
			return m_right->put_image(w, h, imageID);
		}

        if (m_ID != NULL_ID || m_rect.w < w || m_rect.h < h)
			return nullptr;

        
		if (m_rect.w == w && m_rect.h == h){
			m_ID = imageID;
			return this;
		}
		
        unique_ptr<node> left, right;
        
        if (m_rect.w - w > m_rect.h - h){	//Если высота больше ширины, то разбиваем по x
            left  = make_unique<node>(node({m_rect.x, 		m_rect.y, 		w, 				m_rect.h},      m_allocator), m_allocator);
            right = make_unique<node>(node({m_rect.x + w, 	m_rect.y, 		m_rect.w - w, 	m_rect.h},      m_allocator), m_allocator);
		}
        
        else {								//Иначе по y
            left  = make_unique<node>(node({m_rect.x, 		m_rect.y, 		m_rect.w, 		h},             m_allocator), m_allocator);
            right = make_unique<node>(node({m_rect.x, 		m_rect.y + h,	m_rect.w, 		m_rect.h - h},  m_allocator), m_allocator);
		}
        
        m_left  = std::move(left);
        m_right = std::move(right);

		return m_left->put_image(w, h, imageID);
    }

    const rect& node::get_rect() const {
        return m_rect;
    }

    std::size_t node::get_id() const {
        return m_ID;
    }

}//namespace jstd
}//namespace texturing