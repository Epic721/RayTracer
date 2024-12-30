#include "Renderer.h"

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
	for (uint32_t y = 0; y < H; y++) {
		for (uint32_t x = 0; x < W; x++) {
			glm::vec2 coord = {(float)x / W, (float)y / H};
			coord = coord * 2.0f - 1.0f; // [0, 1] -> [-1, 1]
			m_ImageData[(y * W) + x] = PerPixel(coord);
		}
	}
	m_FinalImage->SetData(m_ImageData); // upload data to GPU
}

uint32_t Renderer::PerPixel(glm::vec2 coord) {
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	rayDirection = glm::normalize(rayDirection);
	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
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
	if (discriminant >= 0.0f) {
		return 0xffff0000;
	} else {
		return 0xff000000;
	}
}
