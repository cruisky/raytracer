#include "stdafx.h"
#include "Renderer.h"
#include "RendererConfig.h"
#include "Graphics/Scene.h"
#include "Graphics/Camera.h"
#include "Graphics/Film.h"
#include "Math/Sample.h"

namespace TX {
	Renderer::Renderer(
		const RendererConfig& config,
		std::shared_ptr<Scene> scene,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<Film> film,
		std::shared_ptr<IProgressMonitor> monitor)
		: scene(scene), camera(camera), film(film), monitor_(monitor) {
		ThreadScheduler::Instance()->StartAll();
		// Sample buffer
		sample_buf_ = std::make_unique<CameraSample>(10);	// should be enough to trace a ray
		// Config renderer
		Config(config);
		// Init tiled rendering synchronizer
		thread_sync_.Init(config_.width, config_.height);
	}
	Renderer::~Renderer(){
		Abort();
		ThreadScheduler::Instance()->StopAll();
	}

	Renderer& Renderer::Config(const RendererConfig& config){
		Resize(config.width, config.height);
		tracer_.reset(config.NewTracer());
		sampler_.reset(config.NewSampler());
		// generate sample offset for current tracer
		tracer_->BakeSamples(scene.get(), sample_buf_.get());

config_ = config;
		return *this;
	}

	Renderer& Renderer::Resize(int width, int height) {
		if (config_.width != width || config_.height != height){
			bool wasRunning = this->Running();
			Abort();
			config_.width = width;
			config_.height = height;
			camera->Resize(width, height);
			film->Resize(width, height);
			thread_sync_.Init(width, height);
			if (wasRunning) {
				NewTask();
			}
		}
		return *this;
	}

	bool Renderer::Running() {
		return thread_sync_.Running() && ThreadScheduler::Instance()->taskCount > 0;
	}

	void Renderer::Abort(){
		thread_sync_.Abort();
		ThreadScheduler::Instance()->JoinAll();
	}

	void Renderer::NewTask(){
		if (monitor_) monitor_->Reset(float(config_.samples_per_pixel * thread_sync_.TileCount()));
		film->Clear();
		thread_sync_.Resume();
		for (auto i = 0; i < ThreadScheduler::Instance()->ThreadCount(); i++){
			tasks_.push_back(std::make_shared<RenderTask>(this));
			ThreadScheduler::Instance()->AddTask(Task((Task::Func)&RenderTask::Run, tasks_[i].get()));
		}
	}

	void Renderer::Render(int workerId, RNG& random) {
		// duplicate sample buffer in each thread
		CameraSample sample_buf_dup(*sample_buf_);
		for (int i = 0; thread_sync_.Running() && i < config_.samples_per_pixel; i++){
			// sync threads before and after each sample frame
			thread_sync_.PreRenderSync(workerId);
			RenderTiles(sample_buf_dup, random);
			thread_sync_.PostRenderSync(workerId);

			if (workerId == 0){
				film->ScalePixels();
				thread_sync_.ResetTiles();
			}
		}
		if (workerId == 0){
			film->ScalePixels();
			if (monitor_) monitor_->Finish();
		}
	}

	void Renderer::RenderTiles(CameraSample& sample_buf, RNG& random){
		RenderTile* tile;
		Ray ray;
		Color c;
		while (thread_sync_.NextTile(tile)){
			for (int y = tile->ymin; y < tile->ymax; y++){
				for (int x = tile->xmin; x < tile->xmax; x++){
					if (!thread_sync_.Running()) return;
					sampler_->GetSamples(&sample_buf);
					sample_buf.pix_x = x;
					sample_buf.pix_y = y;
					sample_buf.x += x;
					sample_buf.y += y;
					camera->GenerateRay(&ray, sample_buf);
					tracer_->Trace(scene.get(), ray, sample_buf, random, &c);
					film->Commit(sample_buf.x, sample_buf.y, c);
				}
			}
			if (monitor_) monitor_->UpdateInc();
		}
	}
}
