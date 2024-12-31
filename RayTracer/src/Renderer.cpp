#include "Renderer.h"

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color) {
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t final_color = (a << 24) | (b << 16) | (g << 8) | r;
		return final_color;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height) {
	if (m_FinalImage) {
		if ((m_FinalImage->GetWidth() == width) && (m_FinalImage->GetHeight() == height)) {
			return;
		}
		m_FinalImage->Resize(width, height); // includes resize check and also is optimized so that
		// gpu memory is simply freed and reallocated without deleting the actual object/pointer itself
	} else {
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	delete[] m_ImageData; // doesn't do anything if m_ImageData is already nullptr
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render() {
	// render every pixel that makes up the viewport
	uint32_t H = m_FinalImage->GetHeight();
	uint32_t W = m_FinalImage->GetWidth();
	float aspectRatio = (float)W / H;
	for (uint32_t y = 0; y < H; y++) {
		for (uint32_t x = 0; x < W; x++) {
			glm::vec2 coord = {(float)x / W, (float)y / H};
			coord = coord * 2.0f - 1.0f; // [0, 1] -> [-1, 1]
			coord.x *= aspectRatio;

			glm::vec4 color = PerPixel(coord);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[(y * W) + x] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData); // upload data to GPU
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord) {
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	rayDirection = glm::normalize(rayDirection);
	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
	float radius = 0.5f;

	// (b_x^2 + b_y^2 + b_z^2)t^2 + 2(a_xb_x + a_yb_y + a_zb_z)t + (a_x^2 + a_y^2 + a_z^2) - r^2 = 0
	// a = ray origin
	// b = ray direction
	// r = sphere radius
	// t = hit "distance"
	
	// coord becomes part of our ray direction vector
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayDirection, rayOrigin);
	float c = glm::dot(rayOrigin, rayOrigin) - (radius * radius);
	
	float discriminant = (b * b) - (4.0f * a * c);
	if (discriminant < 0.0f) {
		return glm::vec4(0, 0, 0, 1); // rgba
	}

	// quad formula
	float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
	float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

	glm::vec3 hitPoint = rayOrigin + (rayDirection * closestT);
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDir = glm::normalize(glm::vec3( - 1, -1, -1));

	glm::vec3 sphereColor(1, 0, 1);
	//glm::vec3 sphereColor = (normal * 0.5f) + 0.5f;
	return glm::vec4(sphereColor * glm::dot(-lightDir, normal), 1);
	
	// current: rgba
	// old (ultimate target return format): agbr (endianness)
}
